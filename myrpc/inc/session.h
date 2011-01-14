#ifndef H_MYRPC_SESSION_H
#define H_MYRPC_SESSION_H

#if defined(_WIN32) && !defined(_WIN32_WINNT)
#  define _WIN32_WINNT 0x0600
#endif

#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>
#include <boost/thread/recursive_mutex.hpp>
#include <boost/interprocess/detail/atomic.hpp>
#include "dispatcher_type.h"
#include "callable.h"

namespace msgpack {
namespace myrpc {

class session : public boost::enable_shared_from_this<session> {
public:
    session(boost::asio::io_service& io_service, msgpack::myrpc::shared_dispatcher dispatcher)
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
    msgpack::myrpc::shared_dispatcher dispatcher;
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

} // namespace rpc {
} // namespace msgpack {

#endif // H_MYRPC_SESSION_H
