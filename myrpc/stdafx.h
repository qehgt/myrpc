// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#if defined(_WIN32) && !defined(_WIN32_WINNT)            // Specifies that the minimum required platform is Windows Vista.
#  define _WIN32_WINNT 0x0600     // Change this to the appropriate value to target other versions of Windows.
#endif


#include <stdio.h>
#include <map>
#include <boost/weak_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/thread/condition_variable.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/future.hpp>
#include <boost/thread/recursive_mutex.hpp>
