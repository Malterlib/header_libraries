// Copyright (c) Darrell Wright
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/beached/header_libraries
//

#pragma once

#include "ciso646.h"
#include "daw_cpp_feature_check.h"

#if not defined( DAW_NO_FLATTEN ) and not defined( DAW_HAS_MSVC_LIKE )
#define DAW_ATTRIBUTE_FLATTEN [[gnu::flatten, gnu::always_inline]]
#define DAW_ATTRIBUTE_HIDDEN __attribute__( ( visibility( "hidden" ) ) )
#else
#define DAW_ATTRIBUTE_FLATTEN
#define DAW_ATTRIBUTE_HIDDEN
#endif
