#include "inc/callable.h"
#include "inc/session.h"
#include "inc/callable_imp.h"

namespace msgpack {
namespace myrpc {

callable_imp::~callable_imp()
{
    if (boost::shared_ptr<remove_callable_handler_type> r = weak_session_ptr.lock())
    {
        r->remove_unused_callable(id);
    }
}

msgpack::object callable::get_object()
{
    return c->get_future().get().obj;
}


} // namespace myrpc {
} // namespace msgpack {
