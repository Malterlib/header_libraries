// Copyright (c) Darrell Wright
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/beached/header_libraries
//

#pragma once

#include "../ciso646.h"
#include "daw_function_iterator.h"

namespace daw {
	namespace osi_impl {
		template<typename OutputStream>
		struct osi_callable {
			OutputStream *ptr;

			constexpr osi_callable( OutputStream &os ) noexcept
			  : ptr( &os ) {}

			template<typename T>
			constexpr decltype( auto ) operator( )( T &&value ) const {
				return ( *ptr ) << std::forward<T>( value );
			}

			template<typename T>
			constexpr decltype( auto ) operator( )( T &&value ) {
				return ( *ptr ) << std::forward<T>( value );
			}
		};
	} // namespace osi_impl

	template<typename OutputStream>
	constexpr function_iterator<osi_impl::osi_callable<OutputStream>>
	make_output_stream_iterator( OutputStream &strm ) {
		return { osi_impl::osi_callable<OutputStream>( strm ) };
	}
} // namespace daw
