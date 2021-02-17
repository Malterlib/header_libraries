// Copyright (c) Darrell Wright
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/beached/
//

#pragma once

#if defined( __GNUC__ ) or defined( __clang__ )
#define DAW_LIKELY( Bool ) ( __builtin_expect( !!( Bool ), 1 ) )
#define DAW_UNLIKELY( Bool ) ( __builtin_expect( !!( Bool ), 0 ) )
#elif defined( _MSC_VER )
#define DAW_LIKELY( Bool ) !!( Bool )
#define DAW_UNLIKELY( Bool ) !!( Bool )
#else
#define DAW_LIKELY( Bool ) !!( Bool )
#define DAW_UNLIKELY( Bool ) !!( Bool )
#endif
