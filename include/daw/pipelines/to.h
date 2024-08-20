// Copyright (c) Darrell Wright
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/beached/header_libraries
//

#pragma once

#include "daw/daw_move.h"
#include "range.h"

#include <iterator>
#include <type_traits>

namespace daw::pipelines::pimple {
	template<typename T, std::size_t N>
	void array_test( std::array<T, N> const & );

	// Sentinel type to indicate default template type.  This type will not be
	// used for other purposes
	struct UseTypeDefault {};

	template<template<typename...> typename Container>
	struct ToTemplateTemplateContainer {
		template<Range R>
		[[nodiscard]] DAW_ATTRIB_NOINLINE DAW_CPP23_STATIC_CALL_OP constexpr auto
		operator( )( R &&r ) DAW_CPP23_STATIC_CALL_OP_CONST {
			static_assert(
			  requires( iterator_t<R> it ) { Container( it, it ); },
			  "To requires the container to be constructible from an iterator "
			  "pair" );
			return Container( std::begin( DAW_FWD( r ) ), std::end( DAW_FWD( r ) ) );
		}

		[[nodiscard]] DAW_ATTRIB_INLINE DAW_CPP23_STATIC_CALL_OP constexpr auto
		operator( )( auto &&value ) DAW_CPP23_STATIC_CALL_OP_CONST {
			return ToTemplateTemplateContainer<Container>{ }( maybe_view{ value } );
		}
	};

	template<typename Container>
	struct ToContainer {
		template<Range R>
		[[nodiscard]] DAW_ATTRIB_NOINLINE DAW_CPP23_STATIC_CALL_OP constexpr auto
		operator( )( R &&r ) DAW_CPP23_STATIC_CALL_OP_CONST {
			return Container( std::begin( DAW_FWD( r ) ), std::end( DAW_FWD( r ) ) );
		}

		[[nodiscard]] DAW_ATTRIB_INLINE DAW_CPP23_STATIC_CALL_OP constexpr auto
		operator( )( auto &&value ) DAW_CPP23_STATIC_CALL_OP_CONST {
			return ToContainer<Container>( )( maybe_view{ value } );
		}
	};

	namespace pimpl {
		template<typename value_t, auto Default>
		[[nodiscard]] DAW_ATTRIB_INLINE constexpr auto
		make_array_value( auto &first, auto const &last, std::size_t ) {
			if( first != last ) {
				return *first++;
			}
			if constexpr( std::is_same_v<decltype( Default ),
			                             pimple::UseTypeDefault> ) {
				return value_t{ };
			} else if constexpr( std::is_invocable_v<decltype( Default )> ) {
				return Default( );
			} else {
				return Default;
			}
		}
	} // namespace pimpl

	template<typename Array, auto Default>
	struct ToArray {
		template<Range R>
		[[nodiscard]] DAW_ATTRIB_INLINE DAW_CPP23_STATIC_CALL_OP constexpr auto
		operator( )( R &&r ) DAW_CPP23_STATIC_CALL_OP_CONST {
			constexpr auto sz = std::tuple_size_v<Array>;
			using value_t = typename Array::value_type;
			static_assert(
			  std::convertible_to<range_reference_t<R>, value_t>,
			  "Destination type is not compatible with the Range value types" );
			auto first = std::begin( r );
			auto const last = std::end( r );

			return [&]<std::size_t... Is>( std::index_sequence<Is...> ) {
				return std::array<value_t, sz>{
				  pimpl::make_array_value<value_t, Default>( first, last, Is )... };
			}( std::make_index_sequence<sz>{ } );
		}

		[[nodiscard]] DAW_ATTRIB_INLINE DAW_CPP23_STATIC_CALL_OP constexpr auto
		operator( )( auto &&value ) DAW_CPP23_STATIC_CALL_OP_CONST {
			return ToArray<Array, Default>{ }( maybe_view{ value } );
		}
	}; // namespace pimpl
} // namespace daw::pipelines::pimple

namespace daw::pipelines {
	/// Convert Range to a Container that has deduction guides to get the
	/// value_type from two iterators or a single value
	template<template<typename...> typename Container>
	[[nodiscard]] constexpr auto To( ) {
		return pimple::ToTemplateTemplateContainer<Container>{ };
	}

	/// For fully qualified names, construct a compatible Container, that isn't
	/// std::array, from a Range or a single value
	template<typename Container>
	requires( not requires( Container c ) { pimple::array_test( c ); } )
	  [[nodiscard]] constexpr auto To( ) {
		return pimple::ToContainer<Container>{ };
	}

	/// Construct a std::array from a Range
	/// TODO: add single value method
	template<typename Array, auto Default = pimple::UseTypeDefault{ }>
	requires( requires( Array a ) { pimple::array_test( a ); } )
	  [[nodiscard]] constexpr auto To( ) {
		return pimple::ToArray<Array, Default>{ };
	}
} // namespace daw::pipelines
