// Copyright (c) Darrell Wright
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/beached/header_libraries
//

#pragma once

#include "daw/daw_cpp_feature_check.h"
#include "daw/daw_cxmath.h"
#include "daw/daw_iterator_traits.h"
#include "daw/daw_move.h"

#include <cstddef>
#include <iterator>

namespace daw::pipelines {
	namespace pimpl {
		struct sum_t {
			[[nodiscard]] DAW_CPP23_STATIC_CALL_OP constexpr auto
			operator( )( Range auto &&r ) DAW_CPP23_STATIC_CALL_OP_CONST {
				using value_type = daw::iter_value_t<decltype( std::begin( r ) )>;
				auto sum = value_type{ };
				for( auto const &v : r ) {
					sum = sum + v;
				}
				return sum;
			}
		};

		struct count_t {
			template<typename R>
			[[nodiscard]] DAW_CPP23_STATIC_CALL_OP constexpr std::size_t
			operator( )( R const &r ) DAW_CPP23_STATIC_CALL_OP_CONST {
				if constexpr( not Range<R> ) {
					return 1;
				} else if constexpr( requires { r.size( ); } ) {
					return r.size( );
				} else {
					return static_cast<std::size_t>(
					  std::distance( std::begin( r ), std::end( r ) ) );
				}
			}
		};

		template<typename Fn>
		struct CountIf_t {
			[[no_unique_address]] Fn fn;

			[[nodiscard]] constexpr std::size_t
			operator( )( Range auto const &r ) const {
				std::size_t result = 0;
				for( auto const &v : r ) {
					result += static_cast<std::size_t>( func( v ) );
				}
				return result;
			}
		};
		template<typename Fn>
		CountIf_t( Fn ) -> CountIf_t<Fn>;

		struct SumKahanBabushkaNeumaier_t {
			template<Range R>
			[[nodiscard]] DAW_ATTRIB_NOINLINE DAW_CPP23_STATIC_CALL_OP constexpr auto
			operator( )( R &&r ) DAW_CPP23_STATIC_CALL_OP_CONST {
				using fp_t = range_value_t<R>;
				static_assert( std::is_floating_point_v<fp_t>,
				               "Use of Kahan Babushka Neumaier summation requires the "
				               "range value type to be a floating point type" );
				auto sum = fp_t{ 0.0 };
				auto c = fp_t{ 0.0 };
				for( fp_t input : r ) {
					auto t = sum + input;
					if( daw::cxmath::abs( sum ) >= daw::cxmath::abs( input ) ) {
						c += ( sum - t ) + input;
					} else {
						c += ( input - t ) + sum;
					}
					sum = t;
				}
				return sum + c;
			}
		};
	} // namespace pimpl

	inline constexpr auto Count = pimpl::count_t{ };
	inline constexpr auto Sum = pimpl::sum_t{ };
	inline constexpr auto SumKahanBabushkaNeumaier =
	  pimpl::SumKahanBabushkaNeumaier_t{ };

	[[nodiscard]] constexpr auto CountIf( auto &&fn ) {
		return pimpl::CountIf_t{ DAW_FWD( fn ) };
	};

} // namespace daw::pipelines
