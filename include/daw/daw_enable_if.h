// Copyright (c) Darrell Wright
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/beached/header_libraries
//

#pragma once

#include "daw_cpp_feature_check.h"
#include <cstddef>
#include <type_traits>

namespace daw::enable_if_details {
	template<class...>
	struct conjunction : std::true_type {};

	template<class B1>
	struct conjunction<B1> : B1 {};

	template<class B1, class... Bn>
	struct conjunction<B1, Bn...>
	  : std::conditional<bool( B1::value ), conjunction<Bn...>, B1>::type {};
} // namespace daw::enable_if_details

#if defined( __cpp_concepts )
#if __cpp_concepts >= 201907L
namespace daw {
	template<bool... B>
	requires( ( B and ... ) ) using enable_when_t = std::nullptr_t;

	template<typename... Traits>
	requires(
	  enable_if_details::conjunction<Traits...>::value ) using enable_when_all_t =
	  std::nullptr_t;

	template<bool B, typename T>
	requires( B ) using enable_if_t = T;
} // namespace daw
#endif
#else
namespace daw::enable_if_details {
	template<bool>
	struct enable_if {};

	template<>
	struct enable_if<true> {
		using type = std::nullptr_t;
	};
} // namespace daw::enable_if_details

namespace daw {
	template<bool... B>
	using enable_when_t =
	  typename enable_if_details::enable_if<( B && ... )>::type;

	template<typename... Traits>
	using enable_when_all_t = typename enable_if_details::enable_if<
	  enable_if_details::conjunction<Traits...>::value>::type;
} // namespace daw
#endif
