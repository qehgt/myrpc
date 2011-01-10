// myrpc.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

class dispatcher_type {
public:
    virtual ~dispatcher_type() {}

    virtual void dispatch(msgpack::rpc::request req) = 0;
};
typedef boost::shared_ptr<dispatcher_type> shared_dispatcher;


class boost_message_sendable : public msgpack::rpc::message_sendable {
public:
    boost_message_sendable(boost::asio::ip::tcp::socket& socket) 
        : s(socket) 
      {}

    void send_data(msgpack::sbuffer* sbuf)
    {
        s.send(boost::asio::buffer(sbuf->data(), sbuf->size()));
        ::free(sbuf->data());
        sbuf->release();
    }

    void send_data(msgpack::rpc::auto_vreflife vbuf)
    {
        const struct iovec* vec = vbuf->vector();
        size_t veclen = vbuf->vector_size();

        for(size_t i = 0; i < veclen; ++i)
            s.send(boost::asio::buffer(vec[i].iov_base, vec[i].iov_len));
    }

protected:
    boost::asio::ip::tcp::socket& s;
};

class myecho : public dispatcher_type {
public:
    typedef msgpack::rpc::request request;

	void dispatch(request req);

	void add(request req, int a1, int a2)
	{
		req.result(a1 + a2);
	}

	void echo(request req, const std::string& msg)
	{
		req.result(msg);
	}

	void echo_huge(request req, const msgpack::type::raw_ref& msg)
	{
		req.result(msg);
	}

	void err(request req)
	{
		req.error(std::string("always fail"));
	}
};

void myecho::dispatch(request req)
try {
    std::string method;
    req.method().convert(&method);

    if(method == "add") {
        msgpack::type::tuple<int, int> params;
        req.params().convert(&params);
        add(req, params.get<0>(), params.get<1>());

    } else if(method == "echo") {
        msgpack::type::tuple<std::string> params;
        req.params().convert(&params);
        echo(req, params.get<0>());

    } else if(method == "echo_huge") {
        msgpack::type::tuple<msgpack::type::raw_ref> params;
        req.params().convert(&params);
        echo_huge(req, params.get<0>());

    } else if(method == "err") {
        msgpack::type::tuple<> params;
        req.params().convert(&params);
        err(req);

    } else {
        req.error(msgpack::rpc::NO_METHOD_ERROR);
    }

} catch (msgpack::type_error&) {
    req.error(msgpack::rpc::ARGUMENT_ERROR);
    return;

} catch (std::exception& e) {
    req.error(std::string(e.what()));
    return;
}

class session;
struct data_type {
    int i;
    float f;
};

typedef int session_id_type;
typedef boost::shared_future<data_type> future_data;

class callable {
public:
    callable(session_id_type id, const future_data& future_result, const boost::shared_ptr<session>& session) : 
      id(id), f(future_result), weak_session_ptr(session)
      {}
    ~callable();

    future_data& get_future() { return f; }
    data_type get_data() { 
        return f.get(); 
    }


protected:
    session_id_type id;
    future_data f;
    boost::weak_ptr<session> weak_session_ptr;
};

class session : public boost::enable_shared_from_this<session> {
public:
    session(boost::asio::io_service& io_service, shared_dispatcher dispatcher)
        : socket(io_service),
        dispatcher(dispatcher)
    {
        unpacker.reserve_buffer(max_length);
    }

    boost::asio::ip::tcp::socket& get_socket()
    {
        return socket;
    }

    void start()
    {
        socket.async_read_some(boost::asio::buffer(unpacker.buffer(), max_length),
            boost::bind(&session::handle_read, this,
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred));
    }

    callable create_call();
    void remove_unused_callable(session_id_type id, bool reset_data);

protected:

    void handle_read(const boost::system::error_code& error,
        size_t bytes_transferred);

    void process_message(msgpack::object obj, msgpack::rpc::auto_zone z);

    session_id_type current_id;
    typedef boost::shared_ptr<boost::promise<data_type> > promise_type;
    typedef std::map<session_id_type, promise_type> promise_map_type;

    typedef boost::recursive_mutex mutex_type;
    mutex_type mutex;
    promise_map_type promise_map;


    enum { max_length = 32 * 1024 };
    boost::asio::ip::tcp::socket socket;

    msgpack::unpacker unpacker;
    shared_dispatcher dispatcher;
};


void session::process_message(msgpack::object obj, msgpack::rpc::auto_zone z)
{
    using namespace msgpack;
    using namespace msgpack::rpc;

    msg_rpc rpc;
    obj.convert(&rpc); // ~~~ TODO: try/catch block ?

    switch(rpc.type) 
    {
    case REQUEST: 
        {
            msg_request<object, object> req;
            obj.convert(&req);
            shared_request sr(new request_impl(
                shared_message_sendable(new boost_message_sendable(get_socket())),
                req.msgid, req.method, req.param, z));
            dispatcher->dispatch(request(sr));
        }
        break;

    case RESPONSE: 
        {
            msg_response<object, object> res;
            obj.convert(&res);
            shared_request sr(new request_impl(
                shared_message_sendable(new boost_message_sendable(get_socket())),
                res.msgid, res.result, res.error, z));
            dispatcher->dispatch(request(sr));
        }
        break;

    case NOTIFY: 
        {
            msg_notify<object, object> notify;
            obj.convert(&notify);
            shared_request sr(new request_impl(
                shared_message_sendable(new boost_message_sendable(get_socket())),
                0, notify.method, notify.param, z));
            dispatcher->dispatch(request(sr));
        }
        break;

    default:
        throw msgpack::type_error();
    }
}

void session::handle_read(const boost::system::error_code& error, size_t bytes_transferred)
{
    using namespace msgpack;
    using namespace msgpack::rpc;
    if (!error)
    {
        printf("bytes_transferred = %d\n", int(bytes_transferred));
        // process input data...
        unpacker.buffer_consumed(bytes_transferred);
        msgpack::unpacked result;
        while(unpacker.next(&result)) {
            msgpack::object obj = result.get();

            std::auto_ptr<msgpack::zone> z = result.zone();
            process_message(obj, z);
        }

        socket.async_read_some(boost::asio::buffer(unpacker.buffer(), max_length),
            boost::bind(&session::handle_read, this,
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred));
    }
    else
    {
        socket.shutdown(boost::asio::socket_base::shutdown_both);
        socket.close();
    }
}


callable::~callable()
{
    if(boost::shared_ptr<session> r = weak_session_ptr.lock())
    {
        r->remove_unused_callable(id, !f.has_value());
    }
}

void session::remove_unused_callable(session_id_type id, bool reset_data)
{
    mutex_type::scoped_lock lock(mutex);

    // There is no clients for this 'promise',
    // so, we can setup it to default value before deleting.
    // The only reason to do it is calm debuggers that detect throwning exception in boost library
    if (reset_data)
        promise_map[id]->set_value(data_type());
    promise_map.erase(id);
}

callable session::create_call()
{
    mutex_type::scoped_lock lock(mutex);

    // create new future
    session_id_type id = current_id++;
    promise_type new_promise(new boost::promise<data_type>);
    promise_map[id] = new_promise;
    future_data f(new_promise->get_future());
    return callable(id, f, shared_from_this());
}


int main()
{
    using namespace boost::asio;
    const int PORT = 18811;
    io_service io;

    ip::tcp::acceptor acceptor(io, ip::tcp::endpoint(ip::tcp::v4(), PORT));

    shared_dispatcher dispatcher(new myecho());
    boost::shared_ptr<session> s(new session(io, dispatcher));
    acceptor.accept(s->get_socket());
    s->start();
    boost::thread t(boost::bind(&io_service::run, &io));


    /*
    callable call = s->create_call();
    future_data& f = call.get_future();
    // f.get(); // wait until result
    */

    t.join();
    return 0;
}
