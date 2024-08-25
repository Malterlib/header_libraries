// Copyright (c) Darrell Wright
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/beached/header_libraries
//

#include "daw/deprecated/daw_bounded_string.h"

#include "daw/daw_benchmark.h"
#include "daw/daw_string_view.h"
#include "daw/daw_string_view_fwd.h"
#include "daw/daw_utility.h"

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <string>
#include <vector>

#define FALSE( b ) ( not( b ) )
namespace daw {
	enum class tmp_e { a, b, c };

	constexpr bool is_equal_nc( daw::string_view lhs,
	                            daw::string_view rhs ) noexcept {
		if( lhs.size( ) != rhs.size( ) ) {
			return false;
		}
		bool result = true;
		for( size_t n = 0; n < lhs.size( ); ++n ) {
			result &= ( lhs[n] | ' ' ) == ( rhs[n] | ' ' );
		}
		return result;
	}

	tmp_e tmp_e_from_str( daw::string_view str ) {
		if( is_equal_nc( str, "a" ) ) {
			return tmp_e::a;
		}
		if( is_equal_nc( str, "b" ) ) {
			return tmp_e::b;
		}
		if( is_equal_nc( str, "c" ) ) {
			return tmp_e::c;
		}
		throw std::runtime_error( "unknown http request method" );
	}

	daw::bounded_string do_something( daw::bounded_string str, tmp_e &result ) {
		str = str.substr( 0, str.find_first_of( ' ' ) );
		result = tmp_e_from_str( static_cast<daw::string_view>( str ) );
		return str;
	}

	void daw_bounded_string_contexpr_001( ) {
		daw::bounded_string a = "A test";
		tmp_e result = tmp_e::b;
		auto str = do_something( a, result );
		::Unused( str );
		daw::expecting( result == tmp_e::a );
	}

	void daw_bounded_string_make_bounded_string_it( ) {
		std::string a = "This is a test";
		auto b = daw::make_bounded_string_it( a.begin( ), a.end( ) );

		daw::expecting_message(
		  std::equal( a.begin( ), a.end( ), b.begin( ), b.end( ) ),
		  "string and bounded_string should be equal" );
	}

	void daw_bounded_string_erase_index( ) {
		std::string a = "This is a test";
		auto b = daw::make_bounded_string_it( a.begin( ), a.end( ) );
		a.erase( 2, 2 );
		b.erase( 2, 2 );

		daw::expecting_message(
		  std::equal( a.begin( ), a.end( ), b.begin( ), b.end( ) ),
		  "string and bounded_string should be equal" );
	}

	void daw_bounded_string_make_bounded_string_vector( ) {
		std::string a = "This is a test";
		auto b = std::vector<char>( a.begin( ), a.end( ) );

		auto c = daw::make_bounded_string( b );

		daw::expecting_message(
		  std::equal( a.begin( ), a.end( ), b.begin( ), b.end( ) ),
		  "string and vector should be equal" );
		daw::expecting_message(
		  std::equal( c.begin( ), c.end( ), b.begin( ), b.end( ) ),
		  "bounded_string and vector should be equal" );
	}

	void daw_bounded_string_find_last_of_001( ) {
		static daw::bounded_string const a = "abcdefghijklm";
		std::string const b = "abcdefghijklm";
		auto const pos = a.find_last_of( "ij" );
		auto const pos2 = b.find_last_of( "ij" );
		daw::expecting( pos, pos2 );

		auto const es = a.find_last_of( "lm" );
		auto const es2 = b.find_last_of( "lm" );
		daw::expecting( es, es2 );
	}

	void daw_bounded_string_make_test_001( ) {
		unsigned char const p[] = { 'H', 'e', 'l', 'l', 'o', 0 };
		auto sv =
		  daw::make_bounded_string_it( reinterpret_cast<char const *>( p ),
		                               reinterpret_cast<char const *>( p ) + 5 );
		daw::bounded_string p2 = "Hello";
		daw::expecting( sv, p2 );
	}

	void daw_bounded_string_find_first_of_if_001( ) {
		daw::bounded_string const a = "abcdefghijklm";
		auto pos = a.find_first_of_if( []( auto c ) {
			return c == 'c';
		} );
		daw::expecting( 2U, pos );
	}

	void daw_bounded_string_find_first_not_of_if_001( ) {
		daw::bounded_string const a = "abcdefghijklm";
		auto pos = a.find_first_not_of_if( []( auto c ) {
			return c < 'c';
		} );
		daw::expecting( 2U, pos );
	}

	void daw_bounded_string_find_first_of_001( ) {
		daw::bounded_string const a = "abcdefghijklm";
		auto pos = a.find_first_of( "def" );
		daw::expecting( 3U, pos );
	}

	void daw_bounded_string_find_first_not_of_001( ) {
		daw::bounded_string const a = "abcabfghijklm";
		auto pos = a.find_first_not_of( "abc" );
		daw::expecting( 5U, pos );
	}

	void daw_bounded_string_search_001( ) {
		daw::bounded_string const a = "abcdeaaaijklm";
		auto pos = a.search( "aaa" );
		daw::expecting( 5U, pos );
	}

	void daw_bounded_string_search_last_001( ) {
		daw::bounded_string const a = "abcdeaaaijklm";
		auto pos = a.search_last( "aaa" );
		daw::expecting( 5U, pos );
	}

	void tc001( ) {
		daw::bounded_string view;
		puts( "Constructs an empty string" );

		{ daw::expecting( view.empty( ) ); }
		puts( "Is 0 bytes in size" );

		{ daw::expecting( view.size( ) == 0 ); }
	}

	//----------------------------------------------------------------------------

	void tc002( ) {
		std::string str = "Hello world";
		auto view = static_cast<daw::bounded_string>( str );

		puts( "Constructs a non-empty string" );
		{ daw::expecting( FALSE( view.empty( ) ) ); }

		puts( "Has non-zero size" );
		{ daw::expecting( view.size( ) != 0 ); }
	}

	//----------------------------------------------------------------------------

	void tc003( ) {
		auto empty_str = "";
		auto non_empty_str = "Hello World";

		puts( "Is empty with empty string" );
		{
			daw::bounded_string view = empty_str;

			daw::expecting( view.empty( ) );
		}

		puts( "Is non-empty with non-empty string" );
		{
			daw::bounded_string view = non_empty_str;

			daw::expecting( FALSE( view.empty( ) ) );
		}

		puts( "Is size 0 with empty string" );
		{
			daw::bounded_string view = empty_str;

			daw::expecting( 0U, view.size( ) );
		}

		puts( "Is not size 0 with non-empty string" );
		{
			daw::bounded_string view = non_empty_str;

			daw::expecting( view.size( ) != 0U );
		}
	}

	//----------------------------------------------------------------------------

	void tc004( ) {
		daw::bounded_string empty = "";
		daw::bounded_string view = "Hello world";

		puts( "Is empty with empty string" );
		{ daw::expecting( empty.empty( ) ); }

		puts( "Is non-empty with non-empty string" );
		{ daw::expecting( FALSE( view.empty( ) ) ); }

		puts( "Is size 0 with empty string" );
		{ daw::expecting( 0U, empty.size( ) ); }

		puts( "Is not size 0 with non-empty string" );
		{ daw::expecting( view.size( ) != 0U ); }
	}

	//----------------------------------------------------------------------------
	// Capacity
	//----------------------------------------------------------------------------

	void tc004capacity( ) {
		const char *str = "Hello World";
		daw::bounded_string view = str;

		puts( "Returns non-zero for non-empty string" );
		{ daw::expecting( view.size( ) == std::char_traits<char>::length( str ) ); }

		puts( "Returns 0 for empty string" );
		{
			view = "";

			daw::expecting( 0U, view.size( ) );
		}
	}

	//----------------------------------------------------------------------------

	void tc005capacity( ) {
		const char *str = "Hello World";
		daw::bounded_string view = str;

		puts( "Returns non-zero for non-empty string" );
		{
			daw::expecting( view.length( ) == std::char_traits<char>::length( str ) );
		}

		puts( "Returns 0 for empty string" );
		{
			view = "";

			daw::expecting( 0U, view.length( ) );
		}
	}

	//----------------------------------------------------------------------------

	void tc006capacity( ) {
		const char *str = "Hello World";
		daw::bounded_string view = str;

		puts( "Returns false on non-empty string" );
		{ daw::expecting( FALSE( view.empty( ) ) ); }

		puts( "Returns true on empty string" );
		{
			view = "";

			daw::expecting( view.empty( ) );
		}
	}

	//----------------------------------------------------------------------------
	// Element Access
	//----------------------------------------------------------------------------

	void tc011accessor( ) {
		daw::bounded_string view = "Hello World";

		puts( "Returns reference to first character" );
		{ daw::expecting( view.front( ) == 'H' ); }
	}

	//----------------------------------------------------------------------------

	void tc012accessor( ) {
		daw::bounded_string view = "Hello World";

		puts( "Returns reference to last character" );
		{ daw::expecting( view.back( ) == 'd' ); }
	}

	//----------------------------------------------------------------------------
	// Modifiers
	//----------------------------------------------------------------------------

	void tc014modifier( ) {

		daw::bounded_string view = "Hello World";

		puts( "Removes last 6 characters" );
		{
			view.remove_suffix( 6 );

			daw::expecting( view, "Hello" );
			daw::expecting( view == "Hello" );
		}
	}

	//----------------------------------------------------------------------------

	//----------------------------------------------------------------------------
	// String Operations
	//----------------------------------------------------------------------------

	void tc016conversion( ) {
		daw::bounded_string view = "Hello World";

		std::string string = view.to_string( );

		puts( "Copies view to new location in std::string" );
		{ daw::expecting( view.data( ) != string.data( ) ); }

		puts( "Copied string contains same contents as view" );
		{ daw::expecting( string == "Hello World" ); }
	}

	//----------------------------------------------------------------------------

	void tc017conversion( ) {
		daw::bounded_string view = "Hello World";

		std::string string = static_cast<std::string>( view );

		puts( "Copies view to new location in std::string" );
		{ daw::expecting( view.data( ) != string.data( ) ); }

		puts( "Copied string contains same contents as view" );
		{ daw::expecting( string == "Hello World" ); }
	}

	//----------------------------------------------------------------------------
	// Operations
	//----------------------------------------------------------------------------

	void tc018operation( ) {
		daw::bounded_string const view = "Hello World";

		puts( "Throws std::out_of_range if pos >= view.size()" );
		{
			char result[11];

			daw::expecting_exception<std::out_of_range>( [&]( ) {
				view.copy( result, 11, 11 );
			} );
		}

		puts( "Copies entire string" );
		{
			char result[11];
			view.copy( result, 11 );

			daw::expecting( std::strncmp( result, "Hello World", 11 ) == 0 );
		}

		puts( "Copies remaining characters if count > size" );
		{
			char result[11];
			view.copy( result, 20 );

			daw::expecting( 0, std::strncmp( result, "Hello World", 11 ) );
		}

		puts( "Copies part of the string" );
		{
			char result[11];
			view.copy( result, 5 );

			daw::expecting( 0, std::strncmp( result, "Hello", 5 ) );
		}

		puts( "Copies part of the string, offset from the beginning" );
		{
			char result[11];
			view.copy( result, 10, 6 );

			daw::expecting( 0, std::strncmp( result, "World", 5 ) );
		}

		puts( "Returns number of characters copied" );
		{
			char result[11];

			daw::expecting( 11U, view.copy( result, 20 ) );
		}
	}

	//----------------------------------------------------------------------------

	void tc019operation( ) {
		daw::bounded_string view = "Hello World";

		puts( "Returns the full string when given no args" );
		{
			auto substr = view.substr( );
			daw::expecting( substr == "Hello World" );
		}

		puts( "Returns last part of string" );
		{
			auto substr = view.substr( 6 );
			daw::expecting( substr == "World" );
		}

		puts( "Substring returns at most count characters" );
		{
			auto substr = view.substr( 6, 1 );
			daw::expecting( substr, "W" );
			daw::expecting( substr == "W" );
		}

		puts( "Returns up to end of string if length > size" );
		{
			auto substr = view.substr( 6, 10 );
			daw::expecting( substr == "World" );
		}

		puts( "Throws std::out_of_range if pos > size" );
		daw::expecting_exception<std::out_of_range>( [&view]( ) {
			view.substr( 15 );
		} );
	}

	//----------------------------------------------------------------------------

	void tc020comparison( ) {
		puts( "Returns 0 for identical views" );
		{
			daw::bounded_string view = "Hello World";

			daw::expecting( 0, view.compare( "Hello World" ) );
		}

		puts( "Returns nonzero for different views" );
		{
			daw::bounded_string view = "Hello World";

			daw::expecting( view.compare( "Goodbye World" ) != 0 );
		}

		puts( "Returns > 0 for substring beginning with same string" );
		{
			daw::bounded_string view = "Hello World";

			daw::expecting( view.compare( "Hello" ) > 0 );
		}

		puts( "Returns < 0 for superstring beginning with self" );
		{
			daw::bounded_string view = "Hello";

			daw::expecting( view.compare( "Hello World" ) < 0 );
		}

		puts(
		  "Returns < 0 for same-sized string compared to character greater than "
		  "char" );
		{
			daw::bounded_string view = "1234567";

			daw::expecting( view.compare( "1234667" ) < 0 );
		}

		puts(
		  "Returns > 0 for same-sized string compared to character less than "
		  "char" );
		{
			daw::bounded_string view = "1234567";

			daw::expecting( view.compare( "1234467" ) > 0 );
		}
	}

	//----------------------------------------------------------------------------

	void tc021comparison( ) {
		daw::bounded_string view = "Hello World";

		puts( "Is equal" );
		{
			puts( "Returns true for equal string views" );
			{
				daw::bounded_string view2 = "Hello World";

				daw::expecting( view == view2 );
			}

			puts( "Returns true for equal strings with left char array" );
			{ daw::expecting( "Hello World" == view ); }

			puts( "Returns true for equal strings with right char array" );
			{ daw::expecting( view == "Hello World" ); }

			puts( "Returns true for equal strings with left char ptr" );
			{
				const char *str = "Hello World";
				daw::expecting( str == view );
			}

			puts( "Returns true for equal strings with right char ptr" );
			{
				const char *str = "Hello World";
				daw::expecting( view == str );
			}

			puts( "Returns true for equal strings with left std::string" );
			{
				std::string str = "Hello World";
				daw::expecting( str == view );
			}

			puts( "Returns true for equal strings with right std::string" );
			{
				std::string str = "Hello World";
				daw::expecting( view == str );
			}
		}

		puts( "Is not equal" );
		{
			puts( "Returns false for non-equal string views" );
			{
				daw::bounded_string view2 = "Goodbye World";

				daw::expecting( FALSE( view == view2 ) );
			}

			puts( "Returns false for non-equal strings with left char array" );
			{ daw::expecting( FALSE( ( "Goodbye World" == view ) ) ); }

			puts( "Returns false for non-equal strings with right char array" );
			{ daw::expecting( FALSE( ( view == "Goodbye World" ) ) ); }

			puts( "Returns false for non-equal strings with left char ptr" );
			{
				const char *str = "Goodbye World";
				daw::expecting( FALSE( str == view ) );
			}

			puts( "Returns false for non-equal strings with right char ptr" );
			{
				const char *str = "Goodbye World";
				daw::expecting( FALSE( view == str ) );
			}

			puts( "Returns false for non-equal strings with left std::string" );
			{
				std::string str = "Goodbye World";
				daw::expecting( FALSE( str == view ) );
			}

			puts( "Returns false for non-equal strings with right std::string" );
			{
				std::string str = "Goodbye World";
				daw::expecting( FALSE( view == str ) );
			}
		}
	}

	//----------------------------------------------------------------------------

	void tc022comparison( ) {
		daw::bounded_string view = "Hello World";

		puts( "Is equal" );
		{
			puts( "Returns false for equal string views" );
			{
				daw::bounded_string view2 = "Hello World";

				daw::expecting( FALSE( view != view2 ) );
			}

			puts( "Returns false for equal strings with left char array" );
			{ daw::expecting( FALSE( "Hello World" != view ) ); }

			puts( "Returns false for equal strings with right char array" );
			{ daw::expecting( FALSE( view != "Hello World" ) ); }

			puts( "Returns false for equal strings with left char ptr" );
			{
				const char *str = "Hello World";
				daw::expecting( FALSE( str != view ) );
			}

			puts( "Returns false for equal strings with right char ptr" );
			{
				const char *str = "Hello World";
				daw::expecting( FALSE( view != str ) );
			}

			puts( "Returns false for equal strings with left std::string" );
			{
				std::string str = "Hello World";
				daw::expecting( FALSE( str != view ) );
			}

			puts( "Returns false for equal strings with right std::string" );
			{
				std::string str = "Hello World";
				daw::expecting( FALSE( view != str ) );
			}
		}

		puts( "Is not equal" );
		{
			puts( "Returns true for non-equal string views" );
			{
				daw::bounded_string view2 = "Goodbye World";

				daw::expecting( view != view2 );
			}

			puts( "Returns true for non-equal strings with left char array" );
			{ daw::expecting( "Goodbye World" != view ); }

			puts( "Returns true for non-equal strings with right char array" );
			{ daw::expecting( view != "Goodbye World" ); }

			puts( "Returns true for non-equal strings with left char ptr" );
			{
				const char *str = "Goodbye World";
				daw::expecting( str != view );
			}

			puts( "Returns true for non-equal strings with right char ptr" );
			{
				const char *str = "Goodbye World";
				daw::expecting( view != str );
			}

			puts( "Returns true for non-equal strings with left std::string" );
			{
				std::string str = "Goodbye World";
				daw::expecting( str != view );
			}

			puts( "Returns true for non-equal strings with right std::string" );
			{
				std::string str = "Goodbye World";
				daw::expecting( view != str );
			}
		}
	}

	void daw_bounded_string_split_001( ) {
		daw::bounded_string str = "This is a test of the split";
		auto const str_splt = split( str, ' ' );
		daw::expecting( 7U, str_splt.size( ) );
		std::cout << str << "\n\n";
		std::cout << "items:\n";
		for( auto const &s : str_splt ) {
			std::cout << '"' << s << "\"\n";
		}
		std::cout << "\n\n";
	}

	void daw_bounded_string_split_005( ) {
		std::string b_str;
		daw::bounded_string b = static_cast<daw::bounded_string>( b_str );
		auto const str_splt = split( b, "," );
		daw::expecting( str_splt.empty( ) );
	}

	void daw_can_be_bounded_string_001( ) {
		daw::expecting( daw::can_be_bounded_string<decltype( "Hello" )> );
	}

	void daw_can_be_bounded_string_starts_with_001( ) {
		daw::expecting(
		  daw::bounded_string{ "This is a test" }.starts_with( "This" ) );
	}

	void daw_can_be_bounded_string_starts_with_002( ) {
		daw::expecting( daw::bounded_string{ "This is a test" }.starts_with(
		  daw::bounded_string{ "This" } ) );
	}

	void daw_can_be_bounded_string_starts_with_003( ) {
		daw::expecting(
		  daw::bounded_string{ "This is a test" }.starts_with( 'T' ) );
	}

	void daw_can_be_bounded_string_starts_with_004( ) {
		daw::expecting(
		  !daw::bounded_string{ "This is a test" }.starts_with( "ahis" ) );
	}

	void daw_can_be_bounded_string_starts_with_005( ) {
		daw::expecting( !daw::bounded_string{ "This is a test" }.starts_with(
		  daw::bounded_string{ "ahis" } ) );
	}

	void daw_can_be_bounded_string_starts_with_006( ) {
		daw::expecting(
		  !daw::bounded_string{ "This is a test" }.starts_with( 'a' ) );
	}

	void daw_can_be_bounded_string_ends_with_001( ) {
		daw::expecting(
		  daw::bounded_string{ "This is a test" }.ends_with( "test" ) );
	}

	void daw_can_be_bounded_string_ends_with_002( ) {
		daw::expecting( daw::bounded_string{ "This is a test" }.ends_with(
		  daw::bounded_string{ "test" } ) );
	}

	void daw_can_be_bounded_string_ends_with_003( ) {
		daw::expecting( daw::bounded_string{ "This is a test" }.ends_with( 't' ) );
	}

	void daw_can_be_bounded_string_ends_with_004( ) {
		daw::expecting(
		  !daw::bounded_string{ "This is a test" }.ends_with( "aest" ) );
	}

	void daw_can_be_bounded_string_ends_with_005( ) {
		daw::expecting( !daw::bounded_string{ "This is a test" }.ends_with(
		  daw::bounded_string{ "aest" } ) );
	}

	void daw_can_be_bounded_string_ends_with_006( ) {
		daw::expecting( !daw::bounded_string{ "This is a test" }.ends_with( 'a' ) );
	}

	void daw_bounded_string_constexpr_001( ) {
		constexpr daw::bounded_string a = "This is a test";
		::Unused( a );
	}

	void daw_bounded_string_overfull_001( ) {
		daw::basic_bounded_string<char, 4> a = "This";
		try {
			a.push_back( 'a' );
		} catch( std::out_of_range const & ) { return; } catch( ... ) {
		}

		puts( "Expected out_of_range exception" );
		std::abort( );
	}

	constexpr void cxcopy_test( daw::bounded_string &lhs,
	                            bounded_string const &rhs ) noexcept {
		lhs = rhs;
	}

	constexpr auto cxcopy_cosnt_test( bounded_string const &rhs ) noexcept {
		return rhs;
	}

	constexpr auto cxcopy_tester( ) noexcept {
		daw::bounded_string a = "String a";
		daw::bounded_string b = "String b";
		cxcopy_test( a, b );
		return cxcopy_cosnt_test( a );
	}

	void constexpr_copy_001( ) {
		constexpr auto str = cxcopy_tester( );
		daw::expecting( str, "String b" );
	}

	constexpr bool ctad_test( ) {
		daw::basic_bounded_string blah = "Hello";
		daw::expecting( blah.extent, 6U );
		return true;
	}
	static_assert( ctad_test( ) );

	constexpr bool convert_bigger( ) {
		daw::basic_bounded_string t1 = "Hello";
		auto t2 = static_cast<daw::basic_bounded_string<char, 100>>( t1 );
		return t1 == t2;
	}
	static_assert( convert_bigger( ) );

	void big( ) {
		daw::expecting( convert_bigger( ) );
	}
} // namespace daw

int main( ) {
	daw::daw_bounded_string_contexpr_001( );
	daw::daw_bounded_string_make_bounded_string_it( );
	daw::daw_bounded_string_erase_index( );
	daw::daw_bounded_string_make_bounded_string_vector( );
	daw::daw_bounded_string_find_last_of_001( );
	daw::daw_bounded_string_make_test_001( );
	daw::daw_bounded_string_find_first_of_if_001( );
	daw::daw_bounded_string_find_first_not_of_if_001( );
	daw::daw_bounded_string_find_first_of_001( );
	daw::daw_bounded_string_find_first_not_of_001( );
	daw::daw_bounded_string_search_001( );
	daw::daw_bounded_string_search_last_001( );
	daw::tc001( );
	daw::tc002( );
	daw::tc003( );
	daw::tc004( );
	daw::tc004capacity( );
	daw::tc005capacity( );
	daw::tc006capacity( );
	daw::tc011accessor( );
	daw::tc012accessor( );
	daw::tc014modifier( );
	daw::tc016conversion( );
	daw::tc017conversion( );
	daw::tc018operation( );
	daw::tc019operation( );
	daw::tc020comparison( );
	daw::tc021comparison( );
	daw::tc022comparison( );
	daw::daw_bounded_string_split_001( );
	daw::daw_bounded_string_split_005( );
	daw::daw_can_be_bounded_string_001( );
	daw::daw_can_be_bounded_string_starts_with_001( );
	daw::daw_can_be_bounded_string_starts_with_002( );
	daw::daw_can_be_bounded_string_starts_with_003( );
	daw::daw_can_be_bounded_string_starts_with_004( );
	daw::daw_can_be_bounded_string_starts_with_005( );
	daw::daw_can_be_bounded_string_starts_with_006( );
	daw::daw_can_be_bounded_string_ends_with_001( );
	daw::daw_can_be_bounded_string_ends_with_002( );
	daw::daw_can_be_bounded_string_ends_with_003( );
	daw::daw_can_be_bounded_string_ends_with_004( );
	daw::daw_can_be_bounded_string_ends_with_005( );
	daw::daw_can_be_bounded_string_ends_with_006( );
	daw::daw_bounded_string_constexpr_001( );
	daw::daw_bounded_string_overfull_001( );
	daw::constexpr_copy_001( );
	daw::big( );
}
