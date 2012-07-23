#ifndef H_MYRPC_STREAM_TCP_SOCKET_H
#define H_MYRPC_STREAM_TCP_SOCKET_H

#include <boost/asio.hpp>
#include "interfaces.h"
#include <boost/thread.hpp>

namespace msgpack {
namespace myrpc {

class stream_tcp_socket : public io_stream_object {
public:
    stream_tcp_socket(boost::shared_ptr<boost::asio::io_service> io) 
        : io(io), socket(*io)
    {}

    virtual size_t write(const void* data, size_t size);
    virtual void async_read_some(void* data, size_t size, boost::shared_ptr<read_handler_type> handler);
    virtual boost::system::error_code close(boost::system::error_code& ec);

    boost::asio::ip::tcp::socket& get_socket() {
        return socket;
    }

protected:
    boost::shared_ptr<boost::asio::io_service> io; // to guarantee that 'io' lives at least as this 'stream_tcp_socket'
    boost::asio::ip::tcp::socket socket;

    typedef boost::mutex mutex_type;
    mutex_type mutex;
};

} // namespace rpc {
} // namespace msgpack {

#endif // H_MYRPC_STREAM_TCP_SOCKET_H
