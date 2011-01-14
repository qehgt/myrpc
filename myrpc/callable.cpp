#include "inc/callable.h"
#include "inc/session.h"

namespace msgpack {
namespace myrpc {

callable_type::~callable_type()
{
    if (boost::shared_ptr<session> r = weak_session_ptr.lock())
    {
        r->remove_unused_callable(id, !f.has_value());
    }
}

} // namespace myrpc {
} // namespace msgpack {
