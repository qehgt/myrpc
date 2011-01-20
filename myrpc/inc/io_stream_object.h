#ifndef H_MYRPC_IO_STREAM_OBJECT_H
#define H_MYRPC_IO_STREAM_OBJECT_H

#include <stdlib.h>
#include <boost/system/error_code.hpp>

namespace msgpack {
namespace myrpc {

class read_handler_type {
public:
    virtual ~read_handler_type() {}
    virtual void handle_read(const boost::system::error_code& error, size_t bytes_transferred) = 0;
};

class io_stream_object {
public:
    virtual ~io_stream_object() {}

    virtual size_t write(const void* data, size_t size) = 0; // can throw exceptions
    virtual void async_read_some(void* data, size_t size, read_handler_type* handler) = 0;
    virtual boost::system::error_code close(boost::system::error_code& ec) = 0;
};

} // namespace myrpc {
} // namespace msgpack {

#endif // H_MYRPC_IO_STREAM_OBJECT_H
