#ifndef H_MYRPC_INTERFACES_H
#define H_MYRPC_INTERFACES_H

#include <stdlib.h>
#include <boost/system/error_code.hpp>
#include <boost/shared_ptr.hpp>
#include "request.h"
#include "atomic_ops.h"

namespace msgpack {
namespace myrpc {

class session; // forward declaration

typedef atomic_int_type request_id_type;

class read_handler_type {
public:
    virtual ~read_handler_type() {}
    virtual void handle_read(const boost::system::error_code& error, size_t bytes_transferred) = 0;
};

class io_stream_object {
public:
    virtual ~io_stream_object() {}

    virtual size_t write(const void* data, size_t size) = 0; // can throw exceptions
    virtual void async_read_some(void* data, size_t size, boost::shared_ptr<read_handler_type> handler) = 0;
    virtual boost::system::error_code close(boost::system::error_code& ec) = 0;
};

class remove_callable_handler_type {
public:
    virtual ~remove_callable_handler_type() {}
    virtual void remove_unused_callable(request_id_type id) = 0;
};


class dispatcher_type {
public:
    virtual ~dispatcher_type() {}

    /// Called once during starting of a new session
    virtual void on_start(boost::weak_ptr<session> session_ptr)
    {}

    /// Called when 'session' object is about to destroy
    /// Complicated 'dispatcher' object should free all allocated resources 
    /// and stop all threads
    virtual void on_session_stop()
    {}

    /// Main dispatch routine
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

/// Custom loggers should implement this interface
/// NB: only SEV_ERROR used in myrpc library
class logger_type {
public:
    enum severity_type {
        SEV_EMERGENCY       =   0,   //< system is unusable
        SEV_ALERT           =   1,   //< action must be taken immediately
        SEV_CRITICAL        =   2,   //< critical conditions
        SEV_ERROR           =   3,   //< error conditions
        SEV_WARNING         =   4,   //< warning conditions
        SEV_NOTICE          =   5,   //< normal but significant condition
        SEV_INFORMATIONAL   =   6,   //< informational
        SEV_DEBUG           =   7    //< debug-level messages
    };

    virtual void log(severity_type level, const char* message)
    { // default implementation do nothing
    }
};

} // namespace myrpc {
} // namespace msgpack {

#endif // H_MYRPC_INTERFACES_H
