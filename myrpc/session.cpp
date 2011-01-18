#include "session.h"
#include "message_sendable.h"
#include "request_impl.h"
#include "callable_imp.h"

#include <boost/thread/recursive_mutex.hpp>
#include <boost/thread/mutex.hpp>

namespace msgpack {
namespace myrpc {

class boost_message_sendable : public msgpack::myrpc::message_sendable {
public:
    boost_message_sendable(io_stream_object& stream) 
        : s(stream)
      {}

    void send_data(msgpack::sbuffer* sbuf)
    {
        boost::system::error_code ec;
        s.write(sbuf->data(), sbuf->size(), ec);
        ::free(sbuf->data());
        sbuf->release();

        // TODO: what should we do with socket in case of error?
    }

    void send_data(msgpack::myrpc::auto_vreflife vbuf)
    {
        boost::system::error_code ec;
        const struct iovec* vec = vbuf->vector();
        size_t veclen = vbuf->vector_size();

        for(size_t i = 0; i < veclen; ++i)
            s.write(vec[i].iov_base, vec[i].iov_len, ec);
        // TODO: what should we do with socket in case of error?
    }

protected:
    io_stream_object& s;
};

struct session::session_impl {
    session_impl::session_impl()
    {
        unpacker.reserve_buffer(max_length);
    }

    typedef boost::shared_ptr<boost::promise<msgpack_object_holder> > promise_type;
    typedef std::map<session_id_type, promise_type> promise_map_type;

    typedef boost::recursive_mutex mutex_type;
    mutex_type mutex;
    promise_map_type promise_map;

    enum { max_length = 32 * 1024 };

    msgpack::unpacker unpacker;
};

session::session(boost::shared_ptr<io_stream_object> stream_object, msgpack::myrpc::shared_dispatcher dispatcher)
    : current_id(0), 
    stream(stream_object),
    dispatcher(dispatcher),
    pimpl(new session_impl())
{
}

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
                shared_message_sendable(new boost_message_sendable(*stream)),
                rpc.msgid, rpc.method, rpc.param, z));
            dispatcher->dispatch(request(sr));
        }
        break;

    case RESPONSE: 
        {
            shared_request sr(new request_impl(
                shared_message_sendable(new boost_message_sendable(*stream)),
                rpc.msgid, rpc.method, rpc.param, z));
            process_response(rpc.msgid, rpc.param, z);
        }
        break;

    case NOTIFY: 
        {
            shared_request sr(new request_impl(
                shared_message_sendable(new boost_message_sendable(*stream)),
                0, rpc.method, rpc.param, z));
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
    using namespace msgpack::myrpc;
    if (!error)
    {
        // process input data...
        pimpl->unpacker.buffer_consumed(bytes_transferred);
        msgpack::unpacked result;
        while (pimpl->unpacker.next(&result)) {
            msgpack::object obj = result.get();

            std::auto_ptr<msgpack::zone> z = result.zone();
            process_message(obj, z);
        }

        stream->async_read_some(pimpl->unpacker.buffer(), pimpl->max_length, this);
    }
}

void session::remove_unused_callable(session_id_type id, bool reset_data)
{
    session_impl::mutex_type::scoped_lock lock(pimpl->mutex);

    // There is no clients for this 'promise',
    // so, we can setup it to default value before deleting.
    // The only reason to do it is calm debuggers that detect throwning exception in boost library
    if (reset_data)
        pimpl->promise_map[id]->set_value(msgpack_object_holder());
    pimpl->promise_map.erase(id);
}

void session::process_response(msgpack::myrpc::msgid_t msgid, msgpack::object obj, msgpack::myrpc::auto_zone z)
{
    session_impl::mutex_type::scoped_lock lock(pimpl->mutex);

    session_impl::promise_map_type::iterator i = pimpl->promise_map.find(msgid);
    if (i != pimpl->promise_map.end())
        i->second->set_value(msgpack_object_holder(obj, z));
}

callable session::create_call(session_id_type id)
{
    session_impl::mutex_type::scoped_lock lock(pimpl->mutex);

    // create new future
    session_impl::promise_type new_promise(new boost::promise<msgpack_object_holder>);
    pimpl->promise_map[id] = new_promise;
    future_data f(new_promise->get_future());
    return callable(boost::shared_ptr<callable_type>(new callable_type(id, f, shared_from_this())));
}

void session::start()
{
    stream->async_read_some(pimpl->unpacker.buffer(), pimpl->max_length, this);
}

boost::shared_ptr<io_stream_object> session::get_stream_object()
{
    return stream;
}

} // namespace rpc {
} // namespace msgpack {
