#ifndef H_MYRPC_SESSION_H
#define H_MYRPC_SESSION_H

#if defined(_WIN32) && !defined(_WIN32_WINNT)
#  define _WIN32_WINNT 0x0600
#endif

#include <boost/enable_shared_from_this.hpp>
#include <boost/interprocess/detail/atomic.hpp>
#include "interfaces.h"
#include "callable.h"

namespace msgpack {
namespace myrpc {

struct msgpack_object_holder; // forward declaration

class session : public boost::enable_shared_from_this<session>, protected read_handler_type, public remove_callable_handler_type {
public:
    session(boost::shared_ptr<io_stream_object> stream_object, msgpack::myrpc::shared_dispatcher dispatcher);
    ~session();

    boost::shared_ptr<io_stream_object> get_stream_object();

    void start(on_finish_handler_type* on_finish_handler = NULL);

    inline callable call(const std::string& name);

    inline void notify(const std::string& name);

    template <typename A1>
    inline callable call(const std::string& name,
			const A1& a1);

    template <typename A1, typename A2>
    inline callable call(const std::string& name,
			const A1& a1, const A2& a2);

    template <typename A1, typename A2, typename A3>
    inline callable call(const std::string& name,
			const A1& a1, const A2& a2, const A3& a3);

    template <typename A1, typename A2, typename A3, typename A4>
    inline callable call(const std::string& name,
			const A1& a1, const A2& a2, const A3& a3, const A4& a4);

    template <typename A1, typename A2, typename A3, typename A4, typename A5>
    inline callable call(const std::string& name,
			const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5);

    template <typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
    inline callable call(const std::string& name,
			const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6);

    template <typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7>
    inline callable call(const std::string& name,
			const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6, const A7& a7);

    template <typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8>
    inline callable call(const std::string& name,
			const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6, const A7& a7, const A8& a8);

    template <typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9>
    inline callable call(const std::string& name,
			const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6, const A7& a7, const A8& a8, const A9& a9);

    template <typename A1>
    inline void notify(const std::string& name,
			const A1& a1);

    template <typename A1, typename A2>
    inline void notify(const std::string& name,
			const A1& a1, const A2& a2);

    template <typename A1, typename A2, typename A3>
    inline void notify(const std::string& name,
			const A1& a1, const A2& a2, const A3& a3);

    template <typename A1, typename A2, typename A3, typename A4>
    inline void notify(const std::string& name,
			const A1& a1, const A2& a2, const A3& a3, const A4& a4);

    template <typename A1, typename A2, typename A3, typename A4, typename A5>
    inline void notify(const std::string& name,
			const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5);

    template <typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
    inline void notify(const std::string& name,
			const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6);

    template <typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7>
    inline void notify(const std::string& name,
			const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6, const A7& a7);

    template <typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8>
    inline void notify(const std::string& name,
			const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6, const A7& a7, const A8& a8);

    template <typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9>
    inline void notify(const std::string& name,
			const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6, const A7& a7, const A8& a8, const A9& a9);


protected:
    callable create_call(request_id_type id);
    void process_response(msgpack::myrpc::msgid_t msgid, msgpack::object obj, msgpack::myrpc::auto_zone z);
    void process_error_response(msgpack::myrpc::msgid_t msgid, msgpack::object err, msgpack::myrpc::auto_zone z);

    void handle_read(const boost::system::error_code& error, size_t bytes_transferred);

    void process_message(msgpack::object obj, msgpack::myrpc::auto_zone z);

    void remove_unused_callable(request_id_type id);

    struct session_impl;
    boost::shared_ptr<session_impl> pimpl;

    volatile request_id_type current_id;
    boost::shared_ptr<io_stream_object> stream;
    msgpack::myrpc::shared_dispatcher dispatcher;
    on_finish_handler_type* on_finish_handler;
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

callable session::call(const std::string& name)
{
    request_id_type id = boost::interprocess::detail::atomic_inc32(&current_id);

    msgpack::sbuffer sbuf;
    typedef type::tuple<> Params;
    message_rpc<std::string, Params> msg(myrpc::REQUEST, id, name, Params());

    msgpack::pack(sbuf, msg);
    callable ret = create_call(id);
    stream->write(sbuf.data(), sbuf.size());

    return ret;
}

inline void session::notify(const std::string& name)
{
    using namespace msgpack;
    request_id_type id = boost::interprocess::detail::atomic_inc32(&current_id);

    msgpack::sbuffer sbuf;
    typedef type::tuple<> Params;
    message_rpc<std::string, Params> msg(myrpc::NOTIFY, id, name, Params());

    msgpack::pack(sbuf, msg);
    stream->write(sbuf.data(), sbuf.size());
}

template <typename A1>
inline callable session::call(const std::string& name,
    const A1& a1)
{
    request_id_type id = boost::interprocess::detail::atomic_inc32(&current_id);

    msgpack::sbuffer sbuf;
    typedef type::tuple<A1> Params;
    message_rpc<std::string, Params> msg(myrpc::REQUEST, id, name, Params(a1));

    msgpack::pack(sbuf, msg);
    callable ret = create_call(id);
    stream->write(sbuf.data(), sbuf.size());

    return ret;
}

template <typename A1>
inline void session::notify(const std::string& name,
    const A1& a1)
{
    using namespace msgpack;
    request_id_type id = boost::interprocess::detail::atomic_inc32(&current_id);

    msgpack::sbuffer sbuf;
    typedef type::tuple<A1> Params;
    message_rpc<std::string, Params> msg(myrpc::NOTIFY, id, name, Params(a1));

    msgpack::pack(sbuf, msg);
    stream->write(sbuf.data(), sbuf.size());
}

template <typename A1, typename A2>
inline callable session::call(const std::string& name,
    const A1& a1, const A2& a2)
{
    request_id_type id = boost::interprocess::detail::atomic_inc32(&current_id);

    msgpack::sbuffer sbuf;
    typedef type::tuple<A1, A2> Params;
    message_rpc<std::string, Params> msg(myrpc::REQUEST, id, name, Params(a1, a2));

    msgpack::pack(sbuf, msg);
    callable ret = create_call(id);
    stream->write(sbuf.data(), sbuf.size());

    return ret;
}

template <typename A1, typename A2>
inline void session::notify(const std::string& name,
    const A1& a1, const A2& a2)
{
    using namespace msgpack;
    request_id_type id = boost::interprocess::detail::atomic_inc32(&current_id);

    msgpack::sbuffer sbuf;
    typedef type::tuple<A1, A2> Params;
    message_rpc<std::string, Params> msg(myrpc::NOTIFY, id, name, Params(a1, a2));

    msgpack::pack(sbuf, msg);
    stream->write(sbuf.data(), sbuf.size());
}

template <typename A1, typename A2, typename A3>
inline callable session::call(const std::string& name,
    const A1& a1, const A2& a2, const A3& a3)
{
    request_id_type id = boost::interprocess::detail::atomic_inc32(&current_id);

    msgpack::sbuffer sbuf;
    typedef type::tuple<A1, A2, A3> Params;
    message_rpc<std::string, Params> msg(myrpc::REQUEST, id, name, Params(a1, a2, a3));

    msgpack::pack(sbuf, msg);
    callable ret = create_call(id);
    stream->write(sbuf.data(), sbuf.size());

    return ret;
}

template <typename A1, typename A2, typename A3>
inline void session::notify(const std::string& name,
    const A1& a1, const A2& a2, const A3& a3)
{
    using namespace msgpack;
    request_id_type id = boost::interprocess::detail::atomic_inc32(&current_id);

    msgpack::sbuffer sbuf;
    typedef type::tuple<A1, A2, A3> Params;
    message_rpc<std::string, Params> msg(myrpc::NOTIFY, id, name, Params(a1, a2, a3));

    msgpack::pack(sbuf, msg);
    stream->write(sbuf.data(), sbuf.size());
}

template <typename A1, typename A2, typename A3, typename A4>
inline callable session::call(const std::string& name,
    const A1& a1, const A2& a2, const A3& a3, const A4& a4)
{
    request_id_type id = boost::interprocess::detail::atomic_inc32(&current_id);

    msgpack::sbuffer sbuf;
    typedef type::tuple<A1, A2, A3, A4> Params;
    message_rpc<std::string, Params> msg(myrpc::REQUEST, id, name, Params(a1, a2, a3, a4));

    msgpack::pack(sbuf, msg);
    callable ret = create_call(id);
    stream->write(sbuf.data(), sbuf.size());

    return ret;
}

template <typename A1, typename A2, typename A3, typename A4>
inline void session::notify(const std::string& name,
    const A1& a1, const A2& a2, const A3& a3, const A4& a4)
{
    using namespace msgpack;
    request_id_type id = boost::interprocess::detail::atomic_inc32(&current_id);

    msgpack::sbuffer sbuf;
    typedef type::tuple<A1, A2, A3, A4> Params;
    message_rpc<std::string, Params> msg(myrpc::NOTIFY, id, name, Params(a1, a2, a3, a4));

    msgpack::pack(sbuf, msg);
    stream->write(sbuf.data(), sbuf.size());
}

template <typename A1, typename A2, typename A3, typename A4, typename A5>
inline callable session::call(const std::string& name,
    const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5)
{
    request_id_type id = boost::interprocess::detail::atomic_inc32(&current_id);

    msgpack::sbuffer sbuf;
    typedef type::tuple<A1, A2, A3, A4, A5> Params;
    message_rpc<std::string, Params> msg(myrpc::REQUEST, id, name, Params(a1, a2, a3, a4, a5));

    msgpack::pack(sbuf, msg);
    callable ret = create_call(id);
    stream->write(sbuf.data(), sbuf.size());

    return ret;
}

template <typename A1, typename A2, typename A3, typename A4, typename A5>
inline void session::notify(const std::string& name,
    const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5)
{
    using namespace msgpack;
    request_id_type id = boost::interprocess::detail::atomic_inc32(&current_id);

    msgpack::sbuffer sbuf;
    typedef type::tuple<A1, A2, A3, A4, A5> Params;
    message_rpc<std::string, Params> msg(myrpc::NOTIFY, id, name, Params(a1, a2, a3, a4, a5));

    msgpack::pack(sbuf, msg);
    stream->write(sbuf.data(), sbuf.size());
}

template <typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
inline callable session::call(const std::string& name,
    const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6)
{
    request_id_type id = boost::interprocess::detail::atomic_inc32(&current_id);

    msgpack::sbuffer sbuf;
    typedef type::tuple<A1, A2, A3, A4, A5, A6> Params;
    message_rpc<std::string, Params> msg(myrpc::REQUEST, id, name, Params(a1, a2, a3, a4, a5, a6));

    msgpack::pack(sbuf, msg);
    callable ret = create_call(id);
    stream->write(sbuf.data(), sbuf.size());

    return ret;
}

template <typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
inline void session::notify(const std::string& name,
    const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6)
{
    using namespace msgpack;
    request_id_type id = boost::interprocess::detail::atomic_inc32(&current_id);

    msgpack::sbuffer sbuf;
    typedef type::tuple<A1, A2, A3, A4, A5, A6> Params;
    message_rpc<std::string, Params> msg(myrpc::NOTIFY, id, name, Params(a1, a2, a3, a4, a5, a6));

    msgpack::pack(sbuf, msg);
    stream->write(sbuf.data(), sbuf.size());
}

template <typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7>
inline callable session::call(const std::string& name,
    const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6, const A7& a7)
{
    request_id_type id = boost::interprocess::detail::atomic_inc32(&current_id);

    msgpack::sbuffer sbuf;
    typedef type::tuple<A1, A2, A3, A4, A5, A6, A7> Params;
    message_rpc<std::string, Params> msg(myrpc::REQUEST, id, name, Params(a1, a2, a3, a4, a5, a6, a7));

    msgpack::pack(sbuf, msg);
    callable ret = create_call(id);
    stream->write(sbuf.data(), sbuf.size());

    return ret;
}

template <typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7>
inline void session::notify(const std::string& name,
    const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6, const A7& a7)
{
    using namespace msgpack;
    request_id_type id = boost::interprocess::detail::atomic_inc32(&current_id);

    msgpack::sbuffer sbuf;
    typedef type::tuple<A1, A2, A3, A4, A5, A6, A7> Params;
    message_rpc<std::string, Params> msg(myrpc::NOTIFY, id, name, Params(a1, a2, a3, a4, a5, a6, a7));

    msgpack::pack(sbuf, msg);
    stream->write(sbuf.data(), sbuf.size());
}

template <typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8>
inline callable session::call(const std::string& name,
    const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6, const A7& a7, const A8& a8)
{
    request_id_type id = boost::interprocess::detail::atomic_inc32(&current_id);

    msgpack::sbuffer sbuf;
    typedef type::tuple<A1, A2, A3, A4, A5, A6, A7, A8> Params;
    message_rpc<std::string, Params> msg(myrpc::REQUEST, id, name, Params(a1, a2, a3, a4, a5, a6, a7, a8));

    msgpack::pack(sbuf, msg);
    callable ret = create_call(id);
    stream->write(sbuf.data(), sbuf.size());

    return ret;
}

template <typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8>
inline void session::notify(const std::string& name,
    const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6, const A7& a7, const A8& a8)
{
    using namespace msgpack;
    request_id_type id = boost::interprocess::detail::atomic_inc32(&current_id);

    msgpack::sbuffer sbuf;
    typedef type::tuple<A1, A2, A3, A4, A5, A6, A7, A8> Params;
    message_rpc<std::string, Params> msg(myrpc::NOTIFY, id, name, Params(a1, a2, a3, a4, a5, a6, a7, a8));

    msgpack::pack(sbuf, msg);
    stream->write(sbuf.data(), sbuf.size());
}

template <typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9>
inline callable session::call(const std::string& name,
    const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6, const A7& a7, const A8& a8, const A9& a9)
{
    request_id_type id = boost::interprocess::detail::atomic_inc32(&current_id);

    msgpack::sbuffer sbuf;
    typedef type::tuple<A1, A2, A3, A4, A5, A6, A7, A8, A9> Params;
    message_rpc<std::string, Params> msg(myrpc::REQUEST, id, name, Params(a1, a2, a3, a4, a5, a6, a7, a8, a9));

    msgpack::pack(sbuf, msg);
    callable ret = create_call(id);
    stream->write(sbuf.data(), sbuf.size());

    return ret;
}

template <typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9>
inline void session::notify(const std::string& name,
    const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6, const A7& a7, const A8& a8, const A9& a9)
{
    using namespace msgpack;
    request_id_type id = boost::interprocess::detail::atomic_inc32(&current_id);

    msgpack::sbuffer sbuf;
    typedef type::tuple<A1, A2, A3, A4, A5, A6, A7, A8, A9> Params;
    message_rpc<std::string, Params> msg(myrpc::NOTIFY, id, name, Params(a1, a2, a3, a4, a5, a6, a7, a8, a9));

    msgpack::pack(sbuf, msg);
    stream->write(sbuf.data(), sbuf.size());
}


} // namespace rpc {
} // namespace msgpack {

#endif // H_MYRPC_SESSION_H
