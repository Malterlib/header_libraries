// Copyright (c) Darrell Wright
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/beached/header_libraries
//

#pragma once

#include "daw/ciso646.h"

#include <cstddef>

#if not DAW_HAS_BUILTIN( __type_pack_element )
#include "daw/daw_nth_pack_element.h"
#include "daw_traits_identity.h"
#endif

namespace daw::traits {
#if DAW_HAS_BUILTIN( __type_pack_element )
	template<std::size_t I, typename... Ts>
	using nth_element = __type_pack_element<I, Ts...>;
#else
	template<std::size_t Idx, typename... Ts>
	using nth_element = typename decltype( nth_pack_element<Idx>(
	  daw::traits::identity<Ts>{ }... ) )::type;
#endif
	template<std::size_t Idx, typename... Ts>
	using nth_type = nth_element<Idx, Ts...>;
} // namespace daw::traits
