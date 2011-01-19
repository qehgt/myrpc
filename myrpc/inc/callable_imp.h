#ifndef H_MYRPC_CALLABLE_IMP_H
#define H_MYRPC_CALLABLE_IMP_H

#include <boost/cstdint.hpp>
#include <boost/weak_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/thread/condition_variable.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/future.hpp>
#include "msgpack_header.h"
#include "types.h"
#include "remove_callable_handler.h"

namespace msgpack {
namespace myrpc {

struct msgpack_object_holder {
    msgpack::object obj;
    boost::shared_ptr<msgpack::zone> z;

    msgpack_object_holder() {}
    msgpack_object_holder(msgpack::object o, msgpack::myrpc::auto_zone az)
        : obj(o), z(az.release())
    {
    }
};

typedef boost::shared_future<msgpack_object_holder> future_data;

class callable_imp : public boost::enable_shared_from_this<callable_imp> {
public:
    callable_imp(session_id_type id, const future_data& future_result, const boost::shared_ptr<remove_callable_handler_type>& session) : 
      id(id), f(future_result), weak_session_ptr(session)
      {}
    ~callable_imp();

    future_data& get_future() { return f; }

protected:
    session_id_type id;
    future_data f;
    boost::weak_ptr<remove_callable_handler_type> weak_session_ptr;
};

} // namespace rpc {
} // namespace msgpack {

#endif // H_MYRPC_CALLABLE_IMP_H
