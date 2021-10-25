// Copyright (c) Darrell Wright
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/beached/header_libraries/
//

#pragma once

#include "daw_cpp_feature_check.h"

#include <type_traits>

#if defined( __cpp_nontype_template_args )
#if __cpp_nontype_template_args > 201911L
#define DAW_CXEVAL_FTYPE auto
#else
// Can only pass references/pointers to class types prior to C++20
#define DAW_CXEVAL_FTYPE auto const &
#endif
#endif

namespace daw {
	namespace can_cxeval_impl {
		template<bool, typename T = int>
		using always_t = int;

		template<DAW_CXEVAL_FTYPE F>
		constexpr std::true_type cxeval_test( always_t<( F( ), true )> ) {
			return { };
		}

		template<DAW_CXEVAL_FTYPE>
		constexpr std::false_type cxeval_test( ... ) {
			return { };
		}

		template<DAW_CXEVAL_FTYPE F>
		constexpr auto can_cxeval( ) {
			return cxeval_test<F>( always_t<true>{ } );
		}
	} // namespace can_cxeval_impl

	template<DAW_CXEVAL_FTYPE F>
	using can_constant_evaluate =
	  typename decltype( can_cxeval_impl::can_cxeval<F>( ) )::type;

	template<DAW_CXEVAL_FTYPE F>
	inline constexpr bool can_constant_evaluate_v =
	  can_constant_evaluate<F>::value;
} // namespace daw

#undef DAW_CXEVAL_FTYPE
