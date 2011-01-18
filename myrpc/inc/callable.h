#ifndef H_MYRPC_CALLABLE_H
#define H_MYRPC_CALLABLE_H

#include <boost/shared_ptr.hpp>
#include "msgpack_header.h"
#include "types.h"

namespace msgpack {
namespace myrpc {

class callable_type; // forward declaration

class callable {
public:
    callable(boost::shared_ptr<callable_type> s)
        : c(s)
    {}

    template<typename T>
    T get() { 
        return get_object().as<T>();
    }

protected:
    boost::shared_ptr<callable_type> c;

    msgpack::object get_object();
};

} // namespace rpc {
} // namespace msgpack {

#endif // H_MYRPC_CALLABLE_H
