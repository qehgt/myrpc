// myrpc.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

class dispatcher_type {
public:
    virtual ~dispatcher_type() {}

    virtual void dispatch(msgpack::myrpc::request req) = 0;
};
typedef boost::shared_ptr<dispatcher_type> shared_dispatcher;

class dummy_dispatcher_type : public dispatcher_type {
public:
    void dispatch(msgpack::myrpc::request req)
    {
        req.error(msgpack::myrpc::NO_METHOD_ERROR);
    }
};



class boost_message_sendable : public msgpack::myrpc::message_sendable {
public:
    boost_message_sendable(boost::asio::ip::tcp::socket& socket) 
        : s(socket) 
      {}

    void send_data(msgpack::sbuffer* sbuf)
    {
        boost::system::error_code ec;
        boost::asio::write(s, boost::asio::buffer(sbuf->data(), sbuf->size()),
            boost::asio::transfer_all(), ec);
        ::free(sbuf->data());
        sbuf->release();

        // TODO: what should we do with socket in case of error?
    }

    void send_data(msgpack::myrpc::auto_vreflife vbuf)
    {
        boost::system::error_code ec;
        const struct iovec* vec = vbuf->vector();
        size_t veclen = vbuf->vector_size();

        for(size_t i = 0; i < veclen; ++i) {
            boost::asio::write(s, boost::asio::buffer(vec[i].iov_base, vec[i].iov_len),
                boost::asio::transfer_all(), ec);
        }
        // TODO: what should we do with socket in case of error?
    }

protected:
    boost::asio::ip::tcp::socket& s;
};

class myecho : public dispatcher_type {
public:
    typedef msgpack::myrpc::request request;

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

    if (method == "add") {
        msgpack::type::tuple<int, int> params;
        req.params().convert(&params);
        add(req, params.get<0>(), params.get<1>());

    } else if (method == "echo") {
        msgpack::type::tuple<std::string> params;
        req.params().convert(&params);
        echo(req, params.get<0>());

    } else if (method == "echo_huge") {
        msgpack::type::tuple<msgpack::type::raw_ref> params;
        req.params().convert(&params);
        echo_huge(req, params.get<0>());

    } else if (method == "err") {
        msgpack::type::tuple<> params;
        req.params().convert(&params);
        err(req);

    } else {
        req.error(msgpack::myrpc::NO_METHOD_ERROR);
    }

} catch (msgpack::type_error&) {
    req.error(msgpack::myrpc::ARGUMENT_ERROR);
    return;

} catch (std::exception& e) {
    req.error(std::string(e.what()));
    return;
}

class session;
struct msgpack_object_holder {
    msgpack::object obj;
    boost::shared_ptr<msgpack::zone> z;

    msgpack_object_holder() {}
    msgpack_object_holder(msgpack::object o, msgpack::myrpc::auto_zone az)
        : obj(o), z(az.release())
    {
    }
};

typedef boost::uint32_t session_id_type;
typedef boost::shared_future<msgpack_object_holder> future_data;

class callable_type : public boost::enable_shared_from_this<callable_type> {
public:
    callable_type(session_id_type id, const future_data& future_result, const boost::shared_ptr<session>& session) : 
      id(id), f(future_result), weak_session_ptr(session)
      {}
    ~callable_type();

    future_data& get_future() { return f; }

protected:
    session_id_type id;
    future_data f;
    boost::weak_ptr<session> weak_session_ptr;
};

class callable {
public:
    callable(boost::shared_ptr<callable_type> s)
        : c(s)
    {}

    template<typename T>
    T get() { 
        return c->get_future().get().obj.as<T>();
    }

protected:
    boost::shared_ptr<callable_type> c;
};

class session : public boost::enable_shared_from_this<session> {
public:
    session(boost::asio::io_service& io_service, shared_dispatcher dispatcher)
        : current_id(0),
        socket(io_service),
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

    inline callable call(const std::string& name);

    template <typename A1>
    inline callable call(const std::string& name, const A1& a1);

    template <typename A1, typename A2>
    inline callable call(const std::string& name, const A1& a1, const A2& a2);


    void remove_unused_callable(session_id_type id, bool reset_data);

protected:

    callable create_call(session_id_type id);
    void process_response(msgpack::myrpc::msgid_t msgid, msgpack::object obj, msgpack::myrpc::auto_zone z);

    void handle_read(const boost::system::error_code& error,
        size_t bytes_transferred);

    void process_message(msgpack::object obj, msgpack::myrpc::auto_zone z);

    volatile boost::uint32_t current_id;
    typedef boost::shared_ptr<boost::promise<msgpack_object_holder> > promise_type;
    typedef std::map<session_id_type, promise_type> promise_map_type;

    typedef boost::recursive_mutex mutex_type;
    mutex_type mutex;
    promise_map_type promise_map;

    enum { max_length = 32 * 1024 };
    boost::asio::ip::tcp::socket socket;

    msgpack::unpacker unpacker;
    shared_dispatcher dispatcher;
};

template <typename M, typename P>
struct message_rpc {
	message_rpc() { }

	msgpack::myrpc::message_type_t type;
    msgpack::myrpc::msgid_t msgid;
    M method;
	P param;

    bool is_request()  const { return type == msgpack::myrpc::REQUEST;  }
	bool is_response() const { return type == msgpack::myrpc::RESPONSE; }
	bool is_notify()   const { return type == msgpack::myrpc::NOTIFY;   }

	MSGPACK_DEFINE(type, msgid, method, param);
};


void session::process_message(msgpack::object obj, msgpack::myrpc::auto_zone z)
{
    using namespace msgpack;
    using namespace msgpack::myrpc;

    message_rpc<object, object> rpc;

    try {
      obj.convert(&rpc); // ~~~ TODO: ?
    }
    catch (...) {
        return;
    }

    switch(rpc.type) 
    {
    case REQUEST: 
        {
            shared_request sr(new request_impl(
                shared_message_sendable(new boost_message_sendable(get_socket())),
                rpc.msgid, rpc.method, rpc.param, z));
            dispatcher->dispatch(request(sr));
        }
        break;

    case RESPONSE: 
        {
            shared_request sr(new request_impl(
                shared_message_sendable(new boost_message_sendable(get_socket())),
                rpc.msgid, rpc.method, rpc.param, z));
            process_response(rpc.msgid, rpc.param, z);
        }
        break;

    case NOTIFY: 
        {
            shared_request sr(new request_impl(
                shared_message_sendable(new boost_message_sendable(get_socket())),
                0, rpc.method, rpc.param, z));
            // dispatcher->dispatch(request(sr));
        }
        break;

    default:
        throw msgpack::type_error();
    }
}

void session::handle_read(const boost::system::error_code& error, size_t bytes_transferred)
{
    using namespace msgpack;
    using namespace msgpack::myrpc;
    if (!error)
    {
        // process input data...
        unpacker.buffer_consumed(bytes_transferred);
        msgpack::unpacked result;
        while (unpacker.next(&result)) {
            msgpack::object obj = result.get();

            std::auto_ptr<msgpack::zone> z = result.zone();
            process_message(obj, z);
        }

        socket.async_read_some(boost::asio::buffer(unpacker.buffer(), max_length),
            boost::bind(&session::handle_read, this,
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred));
    }
}


callable_type::~callable_type()
{
    if (boost::shared_ptr<session> r = weak_session_ptr.lock())
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
        promise_map[id]->set_value(msgpack_object_holder());
    promise_map.erase(id);
}

void session::process_response(msgpack::myrpc::msgid_t msgid, msgpack::object obj, msgpack::myrpc::auto_zone z)
{
    mutex_type::scoped_lock lock(mutex);

    promise_map_type::iterator i = promise_map.find(msgid);
    if (i != promise_map.end())
        i->second->set_value(msgpack_object_holder(obj, z));
}

callable session::create_call(session_id_type id)
{
    mutex_type::scoped_lock lock(mutex);

    // create new future
    promise_type new_promise(new boost::promise<msgpack_object_holder>);
    promise_map[id] = new_promise;
    future_data f(new_promise->get_future());
    return callable(boost::shared_ptr<callable_type>(new callable_type(id, f, shared_from_this())));
}

inline callable session::call(const std::string& name)
{
    session_id_type id = boost::interprocess::detail::atomic_inc32(&current_id);

    msgpack::sbuffer sbuf;
    typedef msgpack::type::tuple<> Params;
    message_rpc<std::string, Params> msg;
    msg.type = msgpack::myrpc::REQUEST;
    msg.msgid = id;
    msg.method = name;
    msg.param = Params();

    msgpack::pack(sbuf, msg);
    callable ret = create_call(id);
    get_socket().send(boost::asio::buffer(sbuf.data(), sbuf.size()));

    return ret;
}

template <typename A1>
inline callable session::call(const std::string& name, const A1& a1)
{
    session_id_type id = boost::interprocess::detail::atomic_inc32(&current_id);

    msgpack::sbuffer sbuf;
    typedef msgpack::type::tuple<A1> Params;
    message_rpc<std::string, Params> msg;
    msg.type = msgpack::myrpc::REQUEST;
    msg.msgid = id;
    msg.method = name;
    msg.param = Params(a1);

    msgpack::pack(sbuf, msg);
    callable ret = create_call(id);
    get_socket().send(boost::asio::buffer(sbuf.data(), sbuf.size()));

    return ret;
}

template <typename A1, typename A2>
inline callable session::call(const std::string& name, const A1& a1, const A2& a2)
{
    session_id_type id = boost::interprocess::detail::atomic_inc32(&current_id);

    msgpack::sbuffer sbuf;
    typedef msgpack::type::tuple<A1, A2> Params;
    message_rpc<std::string, Params> msg;
    msg.type = msgpack::myrpc::REQUEST;
    msg.msgid = id;
    msg.method = name;
    msg.param = Params(a1, a2);

    msgpack::pack(sbuf, msg);
    callable ret = create_call(id);
    get_socket().send(boost::asio::buffer(sbuf.data(), sbuf.size()));

    return ret;
}


void run_client_test()
{
    using namespace boost::asio;
    using namespace boost::asio::ip;
    using namespace msgpack;
    const char* PORT = "18811"; // ??

    try {
        io_service io_client;
        tcp::resolver resolver(io_client);
        tcp::resolver::query query(tcp::v4(), "127.0.0.1", PORT);
        tcp::resolver::iterator iterator = resolver.resolve(query);

        shared_dispatcher dispatcher(new dummy_dispatcher_type());
        boost::shared_ptr<session> s(new session(io_client, dispatcher));

        s->get_socket().connect(*iterator);
        s->start();
        boost::thread t(boost::bind(&io_service::run, &io_client));

        callable cc = s->call("add", -12, 13);
        int i = cc.get<int>();
        printf("i = %d\n", i);
        i = s->call("add", 12, 13).get<int>();
        printf("i = %d\n", i);
        std::string str = s->call("echo", std::string("aaaBBB")).get<std::string>();
        printf("str = %s\n", str.c_str());

        // close session
        s.reset();

        t.join();
    }
    catch (const std::exception& e)
    {
        printf("ex: %s\n", e.what());
    }
}


int main()
{
    try {
        using namespace boost::asio;
        const int PORT = 18811;
        io_service io;

        shared_dispatcher dispatcher(new myecho());
        boost::shared_ptr<session> s(new session(io, dispatcher));

        boost::thread t_client(run_client_test);

        ip::tcp::acceptor acceptor(io, ip::tcp::endpoint(ip::tcp::v4(), PORT));
        acceptor.accept(s->get_socket());

        s->start();
        boost::thread t(boost::bind(&io_service::run, &io));

        t.join();
        t_client.join();
    }
    catch (const std::exception& e) {
        printf("main: ex=%s\n", e.what());
    }
    return 0;
}
