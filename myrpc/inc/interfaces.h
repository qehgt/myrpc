#ifndef H_MYRPC_INTERFACES_H
#define H_MYRPC_INTERFACES_H

#include <stdlib.h>
#include <boost/system/error_code.hpp>
#include <boost/shared_ptr.hpp>
#include "session_id.h"
#include "request.h"

namespace msgpack {
namespace myrpc {

class session; // forward declaration

class read_handler_type {
public:
    virtual ~read_handler_type() {}
    virtual void handle_read(const boost::system::error_code& error, size_t bytes_transferred) = 0;
};

class on_finish_handler_type {
public:
    virtual ~on_finish_handler_type () {}
    virtual void on_session_finish(session* s) = 0;
};

class io_stream_object {
public:
    virtual ~io_stream_object() {}

    virtual size_t write(const void* data, size_t size) = 0; // can throw exceptions
    virtual void async_read_some(void* data, size_t size, read_handler_type* handler) = 0;
    virtual boost::system::error_code close(boost::system::error_code& ec) = 0;
};

class remove_callable_handler_type {
public:
    virtual ~remove_callable_handler_type() {}
    virtual void remove_unused_callable(session_id_type id) = 0;
};


class dispatcher_type {
public:
    virtual ~dispatcher_type() {}

    virtual void dispatch(msgpack::myrpc::request req) = 0;
};
typedef boost::shared_ptr<dispatcher_type> shared_dispatcher;

class dummy_dispatcher_type : public dispatcher_type {
public:
    void dispatch(msgpack::myrpc::request req)
    {
        req.error(msgpack::myrpc::NO_METHOD_ERROR);
    }
};


} // namespace myrpc {
} // namespace msgpack {

#endif // H_MYRPC_INTERFACES_H
