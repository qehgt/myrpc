#include "session.h"
#include "message_sendable.h"
#include "request_impl.h"

namespace msgpack {
namespace myrpc {

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

} // namespace rpc {
} // namespace msgpack {
