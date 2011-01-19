#ifndef H_MYRPC_REMOVE_CALLABLE_HANDLER_H
#define H_MYRPC_REMOVE_CALLABLE_HANDLER_H

#include "session_id.h"

namespace msgpack {
namespace myrpc {

class remove_callable_handler_type {
public:
    virtual ~remove_callable_handler_type() {}
    virtual void remove_unused_callable(session_id_type id, bool reset_data) = 0;
};


} // namespace rpc {
} // namespace msgpack {

#endif // H_MYRPC_REMOVE_CALLABLE_HANDLER_H
