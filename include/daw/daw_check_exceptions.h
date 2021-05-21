// Copyright (c) Darrell Wright
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/beached/
//

#pragma once

#include <ciso646>

#if not( defined( __cpp_exceptions ) or defined( __EXCEPTIONS ) or             \
         defined( _CPPUNWIND ) ) or                                            \
  defined( DAW_DONT_USE_EXCEPTIONS )
#ifdef DAW_USE_EXCEPTIONS
#undef DAW_USE_EXCEPTIONS
#endif
#else
#ifndef DAW_USE_EXCEPTIONS
#define DAW_USE_EXCEPTIONS
#endif
#endif
