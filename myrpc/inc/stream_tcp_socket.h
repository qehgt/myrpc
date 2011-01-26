#ifndef H_MYRPC_STREAM_TCP_SOCKET_H
#define H_MYRPC_STREAM_TCP_SOCKET_H

#include <boost/asio.hpp>
#include "interfaces.h"

namespace msgpack {
namespace myrpc {

class stream_tcp_socket : public io_stream_object, public boost::asio::ip::tcp::socket {
public:
    stream_tcp_socket(boost::asio::io_service& io) 
        : boost::asio::ip::tcp::socket(io)
    {}

    virtual size_t write(const void* data, size_t size);
    virtual void async_read_some(void* data, size_t size, read_handler_type* handler);
    virtual boost::system::error_code close(boost::system::error_code& ec);
};

} // namespace rpc {
} // namespace msgpack {

#endif // H_MYRPC_STREAM_TCP_SOCKET_H
