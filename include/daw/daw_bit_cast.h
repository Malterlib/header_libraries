// Copyright (c) Darrell Wright
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/beached/header_libraries
//

#pragma once

#if defined( __has_include )
#if __has_include( <version> )
#include <version>
#endif
#if __has_include( <bit> )
#include <bit>
#endif
#endif
#include <cstring>
#include <ciso646>
#include <type_traits>

namespace daw {
	template<typename To, typename From>
	[[nodiscard, maybe_unused]] To
	bit_cast( From &&from ) noexcept( std::is_nothrow_constructible_v<To> ) {

		static_assert( std::is_trivially_copyable_v<
		                 std::remove_cv_t<std::remove_reference_t<From>>>,
		               "From type must be trivially copiable" );
		static_assert( std::is_trivially_copyable_v<
		                 std::remove_cv_t<std::remove_reference_t<To>>>,
		               "To type must be trivially copiable" );
		static_assert( sizeof( From ) == sizeof( To ),
		               "Sizes of From and To types must be the same" );
		static_assert( std::is_default_constructible_v<To>,
		               "To type must be default constructible" );

		auto result = std::aligned_storage_t<sizeof( To ), alignof( To )>{ };
		return *static_cast<To *>( std::memcpy( &result, &from, sizeof( To ) ) );
	}
} // namespace daw

#if defined( __cpp_lib_bit_cast )
#define DAW_BIT_CAST( type, value ) std::bit_cast<type>( value )
#define DAW_CX_BIT_CAST
#elif defined( __has_builtin ) and __has_builtin( __builtin_bit_cast )
#define DAW_BIT_CAST( type, value ) __builtin_bit_cast( type, value )
#define DAW_CX_BIT_CAST
#elif defined( __clang__ ) and ( __clang_major__ >= 10 )
#define DAW_BIT_CAST( type, value ) __builtin_bit_cast( type, value )
#define DAW_CX_BIT_CAST
#else
#define DAW_BIT_CAST( type, value ) daw::bit_cast<type>( value )
#endif
