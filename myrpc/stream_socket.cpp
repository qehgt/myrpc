#include "io_stream_object.h"
#include <boost/asio.hpp>
#include <boost/thread.hpp>

namespace msgpack {
namespace myrpc {

class stream_socket : public io_stream_object {
public:
    stream_socket(boost::asio::ip::tcp::socket& socket) 
        : s(socket)
    {}

    virtual size_t write(const void* data, size_t size, boost::system::error_code& ec);
    virtual void async_read_some(void* data, size_t size, read_handler_type* handler);

protected:
    boost::asio::ip::tcp::socket& s;
};

size_t stream_socket::write(const void* data, size_t size, boost::system::error_code& ec)
{
    return boost::asio::write(s, boost::asio::buffer(data, size),
        boost::asio::transfer_all(), ec);
}

void stream_socket::async_read_some(void* data, size_t size, read_handler_type* handler)
{
    s.async_read_some(boost::asio::buffer(data, size),
        boost::bind(&read_handler_type::handle_read, handler,
        boost::asio::placeholders::error,
        boost::asio::placeholders::bytes_transferred));
}


} // namespace myrpc {
} // namespace msgpack {
