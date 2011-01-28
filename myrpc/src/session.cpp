#include "inc/session.h"
#include "inc/message_sendable.h"
#include "inc/callable_imp.h"
#include "inc/exception.h"
#include "request_impl.h"

#include <boost/thread/recursive_mutex.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/system/system_error.hpp>

namespace msgpack {
namespace myrpc {

class boost_message_sendable : public msgpack::myrpc::message_sendable {
public:
    boost_message_sendable(io_stream_object& stream) 
        : s(stream)
      {}

    void send_data(msgpack::sbuffer* sbuf)
    {
        try {
            s.write(sbuf->data(), sbuf->size());
        }
        catch (...) {
            ::free(sbuf->data());
            sbuf->release();
            boost::system::error_code ec;
            s.close(ec);
            throw;
        }
        ::free(sbuf->data());
        sbuf->release();
    }

    void send_data(msgpack::myrpc::auto_vreflife vbuf)
    {
        const struct iovec* vec = vbuf->vector();
        size_t veclen = vbuf->vector_size();

        try {
        for(size_t i = 0; i < veclen; ++i)
            s.write(vec[i].iov_base, vec[i].iov_len);
        }
        catch (...) {
            // close socket on error
            boost::system::error_code ec;
            s.close(ec);
            throw;
        }
    }

protected:
    io_stream_object& s;
};

struct session::session_impl {
    session_impl()
    {
        unpacker.reserve_buffer(max_length);
    }

    typedef boost::shared_ptr<boost::promise<msgpack_object_holder> > promise_type;
    typedef std::map<request_id_type, promise_type> promise_map_type;
    typedef std::set<request_id_type> not_used_promises_type;

    typedef boost::recursive_mutex mutex_type;
    mutex_type mutex;
    promise_map_type promise_map;
    not_used_promises_type not_used_promises;

    enum { max_length = 32 * 1024 };

    msgpack::unpacker unpacker;
};

session::session(boost::shared_ptr<io_stream_object> stream_object, msgpack::myrpc::shared_dispatcher dispatcher)
    : current_id(0), 
    stream(stream_object),
    dispatcher(dispatcher),
    pimpl(new session_impl()),
    on_finish_handler(NULL)
{
}

session::~session()
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
            object& error = rpc.method;
            object& result = rpc.param;

            if (error.is_nil()) {
                shared_request sr(new request_impl(
                    shared_message_sendable(new boost_message_sendable(*stream)),
                    rpc.msgid, rpc.method, rpc.param, z));
                process_response(rpc.msgid, rpc.param, z);
            }
            else
                process_error_response(rpc.msgid, error, z);
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

        pimpl->unpacker.reserve_buffer(pimpl->max_length);
        stream->async_read_some(pimpl->unpacker.buffer(), pimpl->max_length, this);
    }
    else {
        boost::system::error_code ec = error;
        stream->close(ec);
        
        // set value for orphaned promises
        if (!pimpl->not_used_promises.empty()) {
            boost::exception_ptr e = boost::copy_exception(boost::system::system_error(error));
            session_impl::mutex_type::scoped_lock lock(pimpl->mutex);

            for (session_impl::not_used_promises_type::iterator i = pimpl->not_used_promises.begin();
                i != pimpl->not_used_promises.end();
                ++i)
            {
                pimpl->promise_map[*i]->set_exception(e);
            }
        }
    }

    if (error && on_finish_handler) on_finish_handler->on_session_finish(this);
}

void session::remove_unused_callable(request_id_type id)
{
    session_impl::mutex_type::scoped_lock lock(pimpl->mutex);
    pimpl->promise_map.erase(id);
}

void session::process_response(msgpack::myrpc::msgid_t msgid, msgpack::object obj, msgpack::myrpc::auto_zone z)
{
    session_impl::mutex_type::scoped_lock lock(pimpl->mutex);

    session_impl::promise_map_type::iterator i = pimpl->promise_map.find(msgid);
    if (i != pimpl->promise_map.end()) {
        pimpl->not_used_promises.erase(msgid);
        i->second->set_value(msgpack_object_holder(obj, z));
    }
}


void session::process_error_response(msgpack::myrpc::msgid_t msgid, msgpack::object err, msgpack::myrpc::auto_zone z)
{
    session_impl::mutex_type::scoped_lock lock(pimpl->mutex);
    session_impl::promise_map_type::iterator i = pimpl->promise_map.find(msgid);
    if (i == pimpl->promise_map.end()) return; // no such id

    pimpl->not_used_promises.erase(msgid);

    if (err.type == msgpack::type::POSITIVE_INTEGER && err.via.u64 == NO_METHOD_ERROR)
        i->second->set_exception(boost::copy_exception(no_method_error()));
	else if (err.type == msgpack::type::POSITIVE_INTEGER && err.via.u64 == ARGUMENT_ERROR)
        i->second->set_exception(boost::copy_exception(argument_error()));
    else {
		std::ostringstream os;
		os << "remote error: ";
		os << err;

        i->second->set_exception(boost::copy_exception(remote_error(os.str())));
    }
}

callable session::create_call(request_id_type id)
{
    session_impl::mutex_type::scoped_lock lock(pimpl->mutex);

    // create new future
    session_impl::promise_type new_promise(new boost::promise<msgpack_object_holder>);
    pimpl->promise_map[id] = new_promise;
    pimpl->not_used_promises.insert(id);
    future_data f(new_promise->get_future());
    return callable(boost::shared_ptr<callable_imp>(new callable_imp(id, f, shared_from_this())));
}

void session::start(on_finish_handler_type* on_finish_handler)
{
    dispatcher->on_start(shared_from_this());
    this->on_finish_handler = on_finish_handler;
    stream->async_read_some(pimpl->unpacker.buffer(), pimpl->max_length, this);
}

boost::shared_ptr<io_stream_object> session::get_stream_object()
{
    return stream;
}

} // namespace rpc {
} // namespace msgpack {
