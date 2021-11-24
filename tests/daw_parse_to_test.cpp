// Copyright (c) Darrell Wright
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/beached/header_libraries
//

#include "daw/daw_benchmark.h"
#include "daw/daw_bounded_array.h"
#include "daw/daw_exception.h"
#include "daw/daw_math.h"
#include "daw/daw_parse_to.h"
#include "daw/daw_string_view.h"
#include "daw/daw_string_view_fwd.h"
#include "daw/daw_utility.h"

#include <cstdint>
#include <sstream>
#include <string>
#include <tuple>

namespace {
	constexpr bool parse_to_000( ) {
		auto vals = daw::parser::parse_to<int, int, int, int>( "0,1,2,3", "," );
		daw::expecting( std::get<0>( vals ), 0 );
		daw::expecting( std::get<1>( vals ), 1 );
		daw::expecting( std::get<2>( vals ), 2 );
		daw::expecting( std::get<3>( vals ), 3 );
		return true;
	}
	static_assert( parse_to_000( ) );

	void daw_parse_to_001( ) {
		auto const vals = daw::parser::parse_to<int, std::string, int, int>(
		  "0,\"hello there\",2,3", "," );
		daw::expecting( std::get<0>( vals ), 0 );
		daw::expecting( std::get<std::string>( vals ), "hello there" );
		daw::expecting( std::get<2>( vals ), 2 );
		daw::expecting( std::get<3>( vals ), 3 );
	}

	constexpr bool parse_to_002( ) {
		daw::sv1::string_view const str = R"(0,"hello there",2,3)";
		auto const vals =
		  daw::parser::parse_to<int, daw::sv1::string_view, int, int>( str, "," );
		daw::expecting( std::get<0>( vals ), 0 );
		daw::expecting( std::get<daw::sv1::string_view>( vals ), "hello there" );
		daw::expecting( std::get<2>( vals ), 2 );
		daw::expecting( std::get<3>( vals ), 3 );
		return true;
	}
	static_assert( parse_to_002( ) );

	namespace daw_values_from_stream_004_ns {
		struct X {
			int a;
			int64_t b;
			uint64_t c;
		};

		constexpr bool from_stream_004( ) {
			auto x = daw::construct_from<X, int, int64_t, uint64_t>(
			  "1,-14334,3434234", "," );
			daw::expecting( x.a, 1 );
			daw::expecting( x.b, -14334LL );
			daw::expecting( x.c, 3434234ULL );
			return true;
		}
		static_assert( from_stream_004( ) );
	} // namespace daw_values_from_stream_004_ns

	constexpr bool parse_to_003( ) {
		auto const f = []( int a, int b, int c ) { return a + b + c; };
		auto result = daw::apply_string2( f, "1,2,3", "," );
		daw::expecting( result, 6 );
		return true;
	}
	static_assert( parse_to_003( ) );

#ifndef WIN32
	constexpr bool parse_to_004( ) {
		auto const f = []( int a, int b, int c ) noexcept { return a + b + c; };
		auto result = daw::apply_string2( f, "1,2,3", "," );
		daw::expecting( result, 6 );
		return true;
	}
	// Workaround
	static_assert( parse_to_004( ) );
#endif

	constexpr bool parse_to_005( ) {
		auto const f = []( int a, int b, int c ) { return a + b + c; };
		auto const result = daw::apply_string<int, int, int>( f, "1,2,3", "," );
		daw::expecting( result, 6 );
		return true;
	}
	static_assert( parse_to_005( ) );

// Workaround
#ifndef WIN32
	constexpr bool parse_to_006( ) {
		auto const f = []( int a, int b, int c ) { return a + b + c; };
		auto const result =
		  daw::apply_string2( f, "1	2  3", daw::parser::whitespace_splitter{ } );
		daw::expecting( result, 6 );
		return true;
	}
	static_assert( parse_to_006( ) );

	constexpr bool parse_to_007( ) {
		auto const f = []( int a, int b, int c ) { return a + b + c; };
		auto const result = daw::apply_string<int, int, int>(
		  f, "1  2     3", daw::parser::whitespace_splitter{ } );
		daw::expecting( result, 6 );
		return true;
	}
	static_assert( parse_to_007( ) );
#endif

	void daw_values_from_stream_001( ) {
		std::string str = "this 1 1.234 test";
		std::stringstream ss{ str };
		auto vals =
		  daw::values_from_stream<daw::parser::converters::unquoted_string, int,
		                          float, daw::parser::converters::unquoted_string>(
		    ss, " " );

		daw::expecting( std::get<0>( vals ), "this" );
		daw::expecting( std::get<1>( vals ), 1 );
		daw::expecting( daw::math::nearly_equal( std::get<2>( vals ), 1.234f ) );
		daw::expecting( std::get<3>( vals ), "test" );
	}

	enum class e_colours : int { red = 2, green = 4, blue = 8 };

	struct invalid_e_colour_exception {};

	constexpr e_colours parse_to_value( daw::sv1::string_view str,
	                                    daw::tag_t<e_colours> ) {
		daw::exception::daw_throw_on_true(
		  str.empty( ), "Attempt to parse an e_colour from an empty string_view" );
		switch( str.front( ) ) {
		case 'r':
		case 'R':
			return e_colours::red;
		case 'g':
		case 'G':
			return e_colours::green;
		case 'b':
		case 'B':
			return e_colours::blue;
		default:
			throw invalid_e_colour_exception{ };
		}
	}

	namespace daw_parse_to_enum_001_ns {

		constexpr int get_value( e_colours colour ) noexcept {
			return static_cast<int>( colour );
		}

		constexpr int sum_colours( e_colours a, e_colours b ) noexcept {
			return get_value( a ) + get_value( b );
		}

		// Workaround
#ifndef WIN32
		constexpr bool parse_to_enum_001( ) {
			auto result = daw::apply_string<e_colours, e_colours>(
			  []( e_colours a, e_colours b ) { return sum_colours( a, b ); },
			  "green blue", daw::parser::whitespace_splitter{ } );
			daw::expecting( result, 12 );
			return true;
		}
		static_assert( parse_to_enum_001( ) );
#endif

		struct callable_t {
			constexpr int operator( )( e_colours a, e_colours b ) const noexcept {
				return sum_colours( a, b );
			}
		};

		constexpr bool parse_to_enum_002( ) {
			auto const result = daw::apply_string<e_colours, e_colours>(
			  callable_t( ), "green blue", " " );

			daw::expecting( result, 12 );
			return true;
		}
		static_assert( parse_to_enum_002( ) );

// Workaround
#ifndef WIN32
		constexpr bool parse_to_enum_003( ) {
			auto result = daw::apply_string<e_colours, e_colours, int>(
			  []( e_colours a, e_colours b, int c ) {
				  return sum_colours( a, b ) + c;
			  },
			  "green blue 534", daw::parser::whitespace_splitter{ } );
			daw::expecting( result, 546 );
			return true;
		}
		static_assert( parse_to_enum_003( ) );

		struct ClassTest {
			int value = 0;

			constexpr ClassTest( ) noexcept = delete;

			constexpr ClassTest( int v ) noexcept
			  : value( v ) {}
		};

		[[nodiscard]] constexpr ClassTest parse_to_value( daw::sv1::string_view str,
		                                                  daw::tag_t<ClassTest> ) {
			return daw::construct_from<ClassTest, int>( str );
		}

		struct e_colours_call {
			constexpr int operator( )( e_colours a, ClassTest b ) const noexcept {
				return static_cast<int>( a ) + b.value;
			}
		};

		constexpr bool parse_to_enum_004( ) {
			constexpr auto result = daw::apply_string<e_colours, ClassTest>(
			  e_colours_call{ }, "green 54", daw::parser::whitespace_splitter{ } );
			daw::expecting( result, 58 );
			return true;
		}
		static_assert( parse_to_enum_004( ) );
#endif
	} // namespace daw_parse_to_enum_001_ns
} // namespace

int main( ) {
	daw_parse_to_001( );
	daw_values_from_stream_001( );
}
