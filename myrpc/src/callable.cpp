#include <boost/date_time.hpp>
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


callable::~callable()
{
}


msgpack::object callable::get_object()
{
    return c->get_future().get().obj;
}


/// Checks to see if the asynchronous result is set.
bool callable::is_ready() const
{
    return c->get_future().is_ready();
}


/// Wait until the result is ready
void callable::wait() const
{
    return c->get_future().wait();
}
    

/// Wait until the result is ready, or the time specified by 'ms' has elapsed
/// @param ms - time in milliseconds
bool callable::timed_wait(unsigned ms)
{
    using namespace boost::posix_time;
    time_duration td = milliseconds(ms);
    
    return c->get_future().timed_wait(td);
}


} // namespace myrpc {
} // namespace msgpack {
