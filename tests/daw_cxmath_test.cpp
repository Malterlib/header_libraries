// Copyright (c) Darrell Wright
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/beached/header_libraries
//

#include "daw/daw_benchmark.h"
#include "daw/daw_cxmath.h"
#include "daw/daw_random.h"

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <limits>
#include <vector>

template<typename Float>
constexpr bool flt_eql_exact( Float lhs, Float rhs ) noexcept {
	// Gets rid of warnings for things we know
	return !( lhs < rhs ) and !( rhs < lhs );
}

static_assert( daw::cxmath::cxmath_impl::bits( 2.0f ).raw_value( ) ==
               0x4000'0000U );
static_assert( daw::cxmath::cxmath_impl::bits( 234324.34375f ).raw_value( ) ==
               0x4864'd516U );
static_assert( daw::cxmath::cxmath_impl::bits( -1.99999988079071044921875f )
                 .raw_value( ) == 0xbfff'ffffU );
static_assert( daw::cxmath::cxmath_impl::bits( 0.0f ).raw_value( ) ==
               0x0000'0000U );
static_assert( flt_eql_exact( daw::cxmath::sqrt( 4.0f ), 2.0f ) );
static_assert( flt_eql_exact( daw::cxmath::copy_sign( 2.0f, 1 ), 2.0f ) );
static_assert( flt_eql_exact( daw::cxmath::copy_sign( 2.0f, -1 ), -2.0f ) );
static_assert( flt_eql_exact( daw::cxmath::copy_sign( -2.0f, -1 ), -2.0f ) );
static_assert( flt_eql_exact( daw::cxmath::copy_sign( -2.0f, 1 ), 2.0f ) );
static_assert( flt_eql_exact( daw::cxmath::fpow2( -1 ), 0.5f ) );
static_assert( flt_eql_exact( daw::cxmath::fpow2( -2 ), 0.25f ) );
static_assert( flt_eql_exact( daw::cxmath::fpow2( 1 ), 2.0f ) );
static_assert( flt_eql_exact( daw::cxmath::fpow2( 2 ), 4.0f ) );
static_assert( flt_eql_exact( daw::cxmath::dpow2( 0 ), 1.0 ) );
static_assert( daw::cxmath::pow10_v<5> == 100000 );

void out_sqrt( float f ) {
	auto result = daw::cxmath::sqrt( f );
	auto E = ( daw::cxmath::sqrt( f * f ) - f ) / f;
	auto cm = std::sqrt( f );
	auto E2 = ( std::sqrt( f * f ) - f ) / f;
	auto diff = result - cm;

	std::cout.precision( std::numeric_limits<float>::max_digits10 );
	std::cout << f << "-> (" << result << ", " << cm << ") E: (" << E << ", "
	          << E2 << ") diff: " << diff << '\n';
}

int main( ) {
	std::cout << "pow10( -1 ) -> " << daw::cxmath::dpow10( -1 ) << '\n';
	std::cout << "pow10( -2 ) -> " << daw::cxmath::dpow10( -2 ) << '\n';
	std::cout << "pow10( -3 ) -> " << daw::cxmath::dpow10( -3 ) << '\n';
	std::cout.precision( std::numeric_limits<float>::max_digits10 );
	out_sqrt( -1.0f );
	out_sqrt( 0.1f );
	out_sqrt( 0.5f );
	out_sqrt( 0.01f );
	out_sqrt( 2.0f );
	out_sqrt( 3.0f );
	out_sqrt( 4.0f );
	out_sqrt( 5.0f );
	out_sqrt( 27.0f );
	out_sqrt( 64.0f );
	out_sqrt( 100.0f );
	out_sqrt( std::numeric_limits<float>::min( ) );
	out_sqrt( std::numeric_limits<float>::max( ) );
	out_sqrt( std::numeric_limits<float>::infinity( ) );
	out_sqrt( -std::numeric_limits<float>::infinity( ) );
	out_sqrt( std::numeric_limits<float>::quiet_NaN( ) );

	auto const nums =
	  daw::make_random_data<int32_t, std::vector<float>>( 1'000, -1'000, 1'000 );
#if defined( DEBUG ) or not defined( NDEBUG )
	constexpr size_t RUNCOUNT = 10'000;
#else
	constexpr size_t RUNCOUNT = 100'000;
#endif
	daw::bench_n_test<RUNCOUNT>(
	  "daw::cxmath::cxmath_impl::bits( flt )",
	  []( auto &&floats ) {
		  float sum = 0.0f;
		  for( auto num : floats ) {
			  sum += daw::cxmath::cxmath_impl::bits( num ).raw_value( );
		  }
		  daw::do_not_optimize( sum );
		  return sum;
	  },
	  nums );

	daw::bench_n_test<RUNCOUNT>(
	  "daw::cxmath::fpow2( int )",
	  []( auto &&floats ) {
		  float sum = 0.0;
		  for( auto num : floats ) {
			  sum += daw::cxmath::fpow2( static_cast<int32_t>( num ) );
		  }
		  daw::do_not_optimize( sum );
		  return sum;
	  },
	  daw::make_random_data<int32_t, std::vector<float>>(
	    1'000, std::numeric_limits<float>::min_exponent10,
	    std::numeric_limits<float>::max_exponent10 ) );

	daw::bench_n_test<RUNCOUNT>(
	  "daw::cxmath::intxp( flt )",
	  []( auto &&floats ) {
		  intmax_t sum = 0.0f;
		  for( auto num : floats ) {
			  sum += *daw::cxmath::intxp( num );
		  }
		  daw::do_not_optimize( sum );
		  return sum;
	  },
	  nums );
	daw::bench_n_test<RUNCOUNT>(
	  "daw::cxmath::setxp( flt, 0 )",
	  []( auto &&floats ) {
		  float sum = 0.0f;
		  for( auto num : floats ) {
			  sum += daw::cxmath::setxp( num, 0 );
		  }
		  daw::do_not_optimize( sum );
		  return sum;
	  },
	  nums );
	daw::bench_n_test<RUNCOUNT>(
	  "daw::cxmath::sqrt( flt )",
	  []( auto &&floats ) {
		  float sum = 0.0f;
		  for( auto num : floats ) {
			  sum += daw::cxmath::sqrt( num );
		  }
		  daw::do_not_optimize( sum );
		  return sum;
	  },
	  nums );
	daw::bench_n_test<RUNCOUNT>(
	  "std::sqrt( flt )",
	  []( auto &&floats ) {
		  float sum = 0.0f;
		  for( auto num : floats ) {
			  sum += std::sqrt( num );
		  }
		  daw::do_not_optimize( sum );
		  return sum;
	  },
	  nums );

	return 0;
}
