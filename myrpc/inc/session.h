#ifndef H_MYRPC_SESSION_H
#define H_MYRPC_SESSION_H

#if defined(_WIN32) && !defined(_WIN32_WINNT)
#  define _WIN32_WINNT 0x0600
#endif

#include <boost/enable_shared_from_this.hpp>
#include <boost/interprocess/detail/atomic.hpp>
#include "dispatcher_type.h"
#include "io_stream_object.h"
#include "callable.h"
#include "io_stream_object.h"
#include "remove_callable_handler.h"

namespace msgpack {
namespace myrpc {

struct msgpack_object_holder; // forward declaration

class session : public boost::enable_shared_from_this<session>, protected read_handler_type, public remove_callable_handler_type {
public:
    session(boost::shared_ptr<io_stream_object> stream_object, msgpack::myrpc::shared_dispatcher dispatcher);

    boost::shared_ptr<io_stream_object> get_stream_object();

    void start();

    inline callable call(const std::string& name);

    template <typename A1>
    inline callable call(const std::string& name, const A1& a1);

    template <typename A1, typename A2>
    inline callable call(const std::string& name, const A1& a1, const A2& a2);

    inline void notify(const std::string& name);

    template <typename A1>
    inline void notify(const std::string& name, const A1& a1);

    template <typename A1, typename A2>
    inline void notify(const std::string& name, const A1& a1, const A2& a2);

protected:
    callable create_call(session_id_type id);
    void process_response(msgpack::myrpc::msgid_t msgid, msgpack::object obj, msgpack::myrpc::auto_zone z);

    void handle_read(const boost::system::error_code& error, size_t bytes_transferred);

    void process_message(msgpack::object obj, msgpack::myrpc::auto_zone z);

    void remove_unused_callable(session_id_type id);

    struct session_impl;
    boost::shared_ptr<session_impl> pimpl;

    volatile session_id_type current_id;
    boost::shared_ptr<io_stream_object> stream;
    msgpack::myrpc::shared_dispatcher dispatcher;
};

template <typename M, typename P>
struct message_rpc {
	message_rpc() { }
	message_rpc(msgpack::myrpc::message_type_t t,
        msgpack::myrpc::msgid_t id,
        M m,
        P p)
        : type(t), msgid(id), method(m), param(p)
    {}

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
    using namespace msgpack;
    session_id_type id = boost::interprocess::detail::atomic_inc32(&current_id);

    msgpack::sbuffer sbuf;
    typedef type::tuple<> Params;
    message_rpc<std::string, Params> msg(myrpc::REQUEST, id, name, Params());

    msgpack::pack(sbuf, msg);
    callable ret = create_call(id);
    stream->write(sbuf.data(), sbuf.size());

    return ret;
}

template <typename A1>
inline callable session::call(const std::string& name, const A1& a1)
{
    session_id_type id = boost::interprocess::detail::atomic_inc32(&current_id);

    msgpack::sbuffer sbuf;
    typedef msgpack::type::tuple<A1> Params;
    message_rpc<std::string, Params> msg(myrpc::REQUEST, id, name, Params(a1));

    msgpack::pack(sbuf, msg);
    callable ret = create_call(id);
    stream->write(sbuf.data(), sbuf.size());

    return ret;
}

template <typename A1, typename A2>
inline callable session::call(const std::string& name, const A1& a1, const A2& a2)
{
    session_id_type id = boost::interprocess::detail::atomic_inc32(&current_id);

    msgpack::sbuffer sbuf;
    typedef msgpack::type::tuple<A1, A2> Params;
    message_rpc<std::string, Params> msg(myrpc::REQUEST, id, name, Params(a1, a2));

    msgpack::pack(sbuf, msg);
    callable ret = create_call(id);
    stream->write(sbuf.data(), sbuf.size());

    return ret;
}

inline void session::notify(const std::string& name)
{
    using namespace msgpack;
    session_id_type id = boost::interprocess::detail::atomic_inc32(&current_id);

    msgpack::sbuffer sbuf;
    typedef type::tuple<> Params;
    message_rpc<std::string, Params> msg(myrpc::NOTIFY, id, name, Params());

    msgpack::pack(sbuf, msg);
    stream->write(sbuf.data(), sbuf.size());
}

template <typename A1>
inline void session::notify(const std::string& name, const A1& a1)
{
    using namespace msgpack;
    session_id_type id = boost::interprocess::detail::atomic_inc32(&current_id);

    msgpack::sbuffer sbuf;
    typedef type::tuple<A1> Params;
    message_rpc<std::string, Params> msg(myrpc::NOTIFY, id, name, Params(a1));

    msgpack::pack(sbuf, msg);
    stream->write(sbuf.data(), sbuf.size());
}

template <typename A1, typename A2>
inline void session::notify(const std::string& name, const A1& a1, const A2& a2)
{
    using namespace msgpack;
    session_id_type id = boost::interprocess::detail::atomic_inc32(&current_id);

    msgpack::sbuffer sbuf;
    typedef type::tuple<A1, A2> Params;
    message_rpc<std::string, Params> msg(myrpc::NOTIFY, id, name, Params(a1, a2));

    msgpack::pack(sbuf, msg);
    stream->write(sbuf.data(), sbuf.size());
}

} // namespace rpc {
} // namespace msgpack {

#endif // H_MYRPC_SESSION_H
