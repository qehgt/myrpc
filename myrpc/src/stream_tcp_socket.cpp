#include "inc/stream_tcp_socket.h"
#include <boost/asio/io_service.hpp>
#include <boost/asio.hpp>
#include <boost/thread.hpp>

namespace msgpack {
namespace myrpc {

size_t stream_tcp_socket::write(const void* data, size_t size)
{
    return boost::asio::write(socket, boost::asio::buffer(data, size),
        boost::asio::transfer_all());
}

void stream_tcp_socket::async_read_some(void* data, size_t size, read_handler_type* handler)
{
    socket.async_read_some(boost::asio::buffer(data, size),
        boost::bind(&read_handler_type::handle_read, handler,
        boost::asio::placeholders::error,
        boost::asio::placeholders::bytes_transferred));
}

boost::system::error_code stream_tcp_socket::close(boost::system::error_code& ec)
{
    return socket.close(ec);
}

} // namespace myrpc {
} // namespace msgpack {
