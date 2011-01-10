// myrpc.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

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
    session(boost::asio::io_service& io_service)
        : socket(io_service)
    {}

    boost::asio::ip::tcp::socket& get_socket()
    {
        return socket;
    }

    void start()
    {
        socket.async_read_some(boost::asio::buffer(buff, max_length),
            boost::bind(&session::handle_read, this,
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred));
    }

    void handle_read(const boost::system::error_code& error,
        size_t bytes_transferred)
    {
        if (!error)
        {
            // process input data...

            /*
            boost::asio::async_write(socket,
                boost::asio::buffer(data, bytes_transferred),
                boost::bind(&session::handle_write, this,
                boost::asio::placeholders::error));
                */
        }
        else
        {
            socket.shutdown(boost::asio::socket_base::shutdown_both);
            socket.close();
        }
    }



    callable create_call();
    void remove_unused_callable(session_id_type id, bool reset_data);

protected:
    session_id_type current_id;
    typedef boost::shared_ptr<boost::promise<data_type> > promise_type;
    typedef std::map<session_id_type, promise_type> promise_map_type;

    typedef boost::recursive_mutex mutex_type;
    mutex_type mutex;
    promise_map_type promise_map;


    enum { max_length = 1024 };
    char buff[max_length];
    boost::asio::ip::tcp::socket socket;
};

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

    boost::shared_ptr<session> s(new session(io));
    acceptor.accept(s->get_socket());
    s->start();
    boost::thread t(boost::bind(&io_service::run, &io));


    /*
    callable call = s->create_call();
    future_data& f = call.get_future();
    // f.get(); // wait until result
    */

    
    //boost::this_thread::sleep(boost::posix_time::seconds(1000));
    t.join();
    return 0;
}
