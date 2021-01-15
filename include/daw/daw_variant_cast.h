// Copyright (c) Darrell Wright
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/beached/header_libraries
//

#pragma once

#include "cpp_17.h"
#include "daw_move.h"
#include "daw_traits.h"
#include "daw_visit.h"

#include <ciso646>
#include <cstddef>
#include <exception>
#include <type_traits>
#include <variant>

namespace daw {
	namespace impl {
		template<typename T>
		struct variant_visitor_t {
			template<typename U,
			         std::enable_if_t<std::is_convertible_v<remove_cvref_t<U>, T>,
			                          std::nullptr_t> = nullptr>
			constexpr T operator( )( U &&result ) const {
				return static_cast<T>( std::forward<U>( result ) );
			}

			template<typename U,
			         std::enable_if_t<not std::is_convertible_v<remove_cvref_t<U>, T>,
			                          std::nullptr_t> = nullptr>
			T operator( )( U && ) const {
				throw std::bad_variant_access( );
			}
		};

		template<typename T>
		struct can_variant_visitor_t {
			constexpr bool operator( )( T const & ) noexcept {
				return true;
			}
			constexpr bool operator( )( T && ) noexcept {
				return true;
			}
			template<typename U>
			constexpr bool operator( )( U const & ) noexcept {
				return false;
			}
		};
	} // namespace impl

	template<typename T, typename... Args>
	constexpr T variant_cast( std::variant<Args...> &var ) {
		static_assert( daw::traits::can_convert_from_v<T, Args...>,
		               "T must be a convertible from type inside of variant" );
		return daw::visit_nt( var, impl::variant_visitor_t<T>{ } );
	}

	template<typename T, typename... Args>
	constexpr T variant_cast( std::variant<Args...> const &var ) {
		static_assert( daw::traits::can_convert_from_v<T, Args...>,
		               "T must be a convertible from type inside of variant" );
		return daw::visit_nt( var, impl::variant_visitor_t<T>{ } );
	}

	template<typename T, typename... Args>
	constexpr T variant_cast( std::variant<Args...> &&var ) {
		static_assert( daw::traits::can_convert_from_v<T, Args...>,
		               "T must be a convertible from type inside of variant" );
		return daw::visit_nt( daw::move( var ), impl::variant_visitor_t<T>{ } );
	}

	template<typename T, typename... Args>
	constexpr bool can_extract( std::variant<Args...> const &var ) noexcept {
		return daw::visit_nt( var, impl::can_variant_visitor_t<T>{ } );
	}

	template<typename T, typename... Args>
	constexpr bool can_extract( std::variant<Args...> &&var ) noexcept {
		return daw::visit_nt( std::move( var ), impl::can_variant_visitor_t<T>{ } );
	}
} // namespace daw
