#ifndef H_MYRPC_CALLABLE_H
#define H_MYRPC_CALLABLE_H

#include <boost/shared_ptr.hpp>
#include "msgpack_header.h"
#include "types.h"

namespace msgpack {
namespace myrpc {

class callable_imp; // forward declaration

class callable {
public:
    callable(boost::shared_ptr<callable_imp> s)
        : c(s)
    {}
    ~callable();

    template<typename T>
    T get() { 
        return get_object().template as<T>();
    }

    void get() {
        get_object().as<msgpack::type::nil>();
    }

    /// Checks to see if the asynchronous result is set.
    bool is_ready() const;
    
    /// Wait until the result is ready
    void wait() const;
    
    /// Wait until the result is ready, or the time specified by 'ms' has elapsed
    /// @param ms - time in milliseconds
    bool timed_wait(unsigned ms);

protected:
    boost::shared_ptr<callable_imp> c;

    msgpack::object get_object();
};


} // namespace rpc {
} // namespace msgpack {

#endif // H_MYRPC_CALLABLE_H
