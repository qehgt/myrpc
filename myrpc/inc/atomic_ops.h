#ifndef H_MYRPC_ATOMIC_OPS_H
#define H_MYRPC_ATOMIC_OPS_H

/// @file Declaration of type/functions for atomic increment/decrement
/// For all platforms:
///   atomic_int_type i = 42;
///   atomic_int_type j = atomic_increment(&i);
///   assert(i==43 && j==42);

#if defined(__GNUC__) && ((__GNUC__*10 + __GNUC_MINOR__) < 41)
#  include <bits/atomicity.h>
#elif defined(__GNUC__) && ((__GNUC__*10 + __GNUC_MINOR__) >= 41)
# // use GCC builtins 
#elif defined(_WIN32) // use 'InterlockedIncrement' function
#  define WIN32_LEAN_AND_MEAN
#  define VC_EXTRALEAN
#  include <windows.h>
#else
#  include <boost/cstdint.hpp>
#  include <boost/interprocess/detail/atomic.hpp>
#endif

namespace msgpack {
namespace myrpc {

#if defined(__GNUC__) && ((__GNUC__*10 + __GNUC_MINOR__) < 41)

typedef int atomic_int_type;

static inline
atomic_int_type atomic_increment(volatile atomic_int_type* p)
{
  return  __gnu_cxx::__exchange_and_add(p, 1);
}

#elif defined(__GNUC__) && ((__GNUC__*10 + __GNUC_MINOR__) >= 41)

typedef int atomic_int_type;

static inline
atomic_int_type atomic_increment(volatile atomic_int_type* p)
{
  return __sync_fetch_and_add(p, 1);
}

#elif defined(_WIN32) // use 'InterlockedIncrement' function
typedef LONG atomic_int_type;

static inline
atomic_int_type atomic_increment(volatile atomic_int_type* p)
{
  return InterlockedIncrement(p) - 1;
}

#else

typedef boost::uint32_t atomic_int_type;

static inline
atomic_int_type atomic_increment(volatile atomic_int_type* p)
{
  return boost::interprocess::detail::atomic_inc32(p);
}

#endif

} // namespace myrpc {
} // namespace msgpack {

#endif // H_MYRPC_ATOMIC_OPS_H
