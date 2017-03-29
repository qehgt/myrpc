#include "inc/session.h"
#include "inc/message_sendable.h"
#include "inc/callable_imp.h"
#include "inc/exception.h"
#include "request_impl.h"

#include <boost/thread/recursive_mutex.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/system/system_error.hpp>
#include <boost/make_shared.hpp>

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
    session_impl(msgpack::myrpc::shared_dispatcher dispatcher,
        boost::shared_ptr<logger_type> logger)
    :
    dispatcher(dispatcher), 
    logger(logger)
    {
        unpacker.reserve_buffer(max_length);
        if (!this->logger) this->logger = boost::make_shared<logger_type>();
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
    shared_dispatcher dispatcher;
    boost::shared_ptr<logger_type> logger;
};

session::session(boost::shared_ptr<io_stream_object> stream_object, msgpack::myrpc::shared_dispatcher dispatcher,
                 boost::shared_ptr<logger_type> logger)
    :
    pi(new session_impl(dispatcher, logger)),
    current_id(0), 
    stream(stream_object)
{
}

session::~session()
{
    try{
        if (pi->dispatcher)
            pi->dispatcher->on_session_stop();
    }
    catch(const std::exception& e)
    {
        pi->logger->log(pi->logger->SEV_ERROR, ("Unhandled exception in on_session_stop(): " + 
            boost::diagnostic_information(e)).c_str());
    }
}

void session::process_message(msgpack::object obj, msgpack::myrpc::auto_zone z)
{
    using namespace msgpack;
    using namespace msgpack::myrpc;

    msg_rpc rpc;

    try {
      obj.convert(rpc);
    }
    catch (const std::exception& e) {
        pi->logger->log(pi->logger->SEV_ERROR, ("msgpack convert() error: " + 
            boost::diagnostic_information(e)).c_str());
        return;
    }

    switch(rpc.type) 
    {
    case REQUEST: 
        {
            msg_request<object, object> req;
            obj.convert(req);
            shared_request sr(new request_impl(
                shared_message_sendable(new boost_message_sendable(*stream)),
                req.msgid, req.method, req.param, z));
            pi->dispatcher->dispatch(request(sr));
        }
        break;

    case RESPONSE: 
        {
            msg_response<object, object> res;
            obj.convert(res);

            if (res.error.is_nil())
                process_response(res.msgid, res.result, z);
            else
                process_error_response(res.msgid, res.error, z);
        }
        break;

    case NOTIFY: 
        {
            msg_notify<object, object> notify;
            obj.convert(notify);
            shared_request sr(new request_impl(
                shared_message_sendable(new boost_message_sendable(*stream)),
                0, notify.method, notify.param, z));
            pi->dispatcher->dispatch(request(sr));
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
    bool failed = false;
    if (!error)
    {
        try {
            // process input data...
            pi->unpacker.buffer_consumed(bytes_transferred);
            msgpack::unpacked result;
            while (pi->unpacker.next(result)) {
                msgpack::object obj = result.get();

                std::auto_ptr<msgpack::zone> z = result.zone();
                process_message(obj, z);
            }

            pi->unpacker.reserve_buffer(pi->max_length);
            stream->async_read_some(pi->unpacker.buffer(), pi->max_length, shared_from_this());
            return;
        }
        catch(std::exception& e)
        {
            pi->logger->log(pi->logger->SEV_ERROR, ("msgpack handle_read() error: " + 
                boost::diagnostic_information(e)).c_str());
            failed = true;
        }
    }
    if (error || failed) {
        boost::system::error_code ec = error;
        stream->close(ec);

        // set value for orphaned promises
        session_impl::mutex_type::scoped_lock lock(pi->mutex);
        if (!pi->not_used_promises.empty()) {
            boost::exception_ptr e = boost::copy_exception(boost::system::system_error(error));

            for (session_impl::not_used_promises_type::iterator i = pi->not_used_promises.begin();
                i != pi->not_used_promises.end();
                ++i)
            {
                pi->promise_map[*i]->set_exception(e);
            }
        }
    }
}

void session::remove_unused_callable(request_id_type id)
{
    session_impl::mutex_type::scoped_lock lock(pi->mutex);
    pi->not_used_promises.erase(id);
    pi->promise_map.erase(id);
}

void session::process_response(msgpack::myrpc::msgid_t msgid, msgpack::object obj, msgpack::myrpc::auto_zone z)
{
    session_impl::mutex_type::scoped_lock lock(pi->mutex);

    session_impl::promise_map_type::iterator i = pi->promise_map.find(msgid);
    if (i != pi->promise_map.end()) {
        pi->not_used_promises.erase(msgid);
        i->second->set_value(msgpack_object_holder(obj, z));
    }
}


void session::process_error_response(msgpack::myrpc::msgid_t msgid, msgpack::object err, msgpack::myrpc::auto_zone z)
{
    session_impl::mutex_type::scoped_lock lock(pi->mutex);
    session_impl::promise_map_type::iterator i = pi->promise_map.find(msgid);
    if (i == pi->promise_map.end()) return; // no such id

    pi->not_used_promises.erase(msgid);

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
    session_impl::mutex_type::scoped_lock lock(pi->mutex);

    // create new future
    session_impl::promise_type new_promise(new boost::promise<msgpack_object_holder>);
    pi->promise_map[id] = new_promise;
    pi->not_used_promises.insert(id);
    future_data f(new_promise->get_future());
    return callable(boost::shared_ptr<callable_imp>(new callable_imp(id, f, shared_from_this())));
}

void session::start()
{
    try {
        pi->dispatcher->on_start(shared_from_this());
    }
    catch (const boost::exception& e)
    {
        try {
            pi->logger->log(pi->logger->SEV_ERROR, ("Unhandled exception in on_start(): " + 
                boost::diagnostic_information(e)).c_str());

            notify("error", boost::diagnostic_information(e));
            pi->dispatcher.reset();
        }
        catch(const std::exception& e)
        {
            pi->logger->log(pi->logger->SEV_ERROR, boost::diagnostic_information(e).c_str());
        }
        return;
    }
    catch (const std::exception& e)
    {
        try {
            notify("error", boost::diagnostic_information(e));
            pi->dispatcher.reset();
        }
        catch(const std::exception& e)
        {
            pi->logger->log(pi->logger->SEV_ERROR, boost::diagnostic_information(e).c_str());
        }
        return;
    }
    stream->async_read_some(pi->unpacker.buffer(), pi->max_length, shared_from_this());
}

boost::shared_ptr<io_stream_object> session::get_stream_object()
{
    return stream;
}

} // namespace rpc {
} // namespace msgpack {
