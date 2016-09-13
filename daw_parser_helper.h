// The MIT License (MIT)
//
// Copyright (c) 2014-2016 Darrell Wright
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files( the "Software" ), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#pragma once

#include <algorithm>
#include <exception>
#include <functional>
#include <list>
#include <vector>
#include <utility>

#include "daw_traits.h"

namespace daw {
	namespace parser {
		template<typename T, typename Iterator>
			struct parser_result {
				T result;
				Iterator first;
				Iterator last;
			};    // parser_result

		struct ParserException: public std::exception { };
		struct ParserEmptyException: public ParserException { };
		struct ParserOverflowException: public ParserException { };
		struct ParserOutOfRangeException: public ParserException, public std::out_of_range {
			template<typename String>
				ParserOutOfRangeException( String && arg ):
					ParserException{ },
				std::out_of_range{ std::forward<String>( arg ) } { }
		};

		template<typename ForwardIterator>
			struct find_result_t {
				ForwardIterator first;
				ForwardIterator last;
				bool found;

				find_result_t( ForwardIterator First, ForwardIterator Last, bool Found ):
					first{ First },
					last{ Last },
					found{ Found } { }

				explicit operator bool( ) const {
					return found;
				}

				bool empty( ) noexcept {
					return first == last;
				}

				bool empty( bool throw_on_empty ) {
					if( throw_on_empty ) {
						throw ParserEmptyException{ };
					}
				}

				template<typename Container>
					auto as( ) const {
						Container result;
						std::copy( first, last, std::back_inserter( result ) );
						return result;
					}

				///
				// Depreciated, use to_string
				std::string as_string( ) const {
					return as<std::string>( );
				}

				std::string to_string( ) const {
					return as<std::string>( );
				}

				auto begin( ) {
					return first;
				}

				auto end( ) {
					return last;
				}
			};    // find_result_t

		template<typename ForwardIterator>
			find_result_t<ForwardIterator> make_find_result( ForwardIterator first, ForwardIterator last, bool result = false ) {
				return find_result_t<ForwardIterator>{ first, last, result };
			}

		template<typename ForwardIterator>
			auto until( ForwardIterator first, ForwardIterator last, std::function<bool(daw::traits::root_type_t<decltype(*first)>)> is_last ) {

				auto result = make_find_result( first, last );
				for( auto it = first; it != last; ++it ) {
					if( (result.found = is_last( *it )) ) {
						result.last = it;
						return result;
					}
				}
				return result;
			}

		template<typename ForwardIterator>
			auto until_false( ForwardIterator first, ForwardIterator last, std::function<bool(daw::traits::root_type_t<decltype(*first)>)> is_last ) {

				auto result = make_find_result( first, last );
				for( auto it = first; it != last; ++it ) {
					if( (result.found = !is_last( *it )) ) {
						result.last = it;
						return result;
					}
				}
				return result;
			}


		template<typename T, typename Arg>
			bool is_a( T && value, Arg && tst ) {
				//static_assert( daw::traits::is_comparable_v<T, Arg>, "value is not comparable to tst" );
				using val_t = typename daw::traits::max_sizeof<T, Arg>::type;
				return static_cast<val_t>(value) == static_cast<val_t>(tst);
			}

		template<typename T, typename Arg, typename... Args>
			bool is_a( T && value, Arg && tst, Args && ... tsts ) {
				return is_a( std::forward<T>( value ), std::forward<Arg>( tst ) ) || is_a( std::forward<T>( value ), std::forward<Args>( tsts )... );
			}


		template<typename Arg>
			class one_of_t {
				std::vector<Arg> m_args;

				public:
				one_of_t( std::initializer_list<Arg> args ):
					m_args{ args } { }

				template<typename T>
					bool operator( )( T && value ) const {
						return value_in( std::forward<T>(value), m_args );
					}
			};	// one_of

		template<typename... Arg>
			auto one_of( Arg&&... args ) {
				return one_of_t<Arg...>{ std::forward<Arg>(args)... };
			}

		template<typename ForwardIterator, typename Value, typename... Values>
			find_result_t<ForwardIterator> until_value( ForwardIterator first, ForwardIterator last, Value && value, Values && ... values ) {
				auto result = make_find_result( first, last );
				for( auto it = first; it != last; ++it ) {
					if( is_a( *it, std::forward<Value>( value ), std::forward<Values>( values )... ) ) {
						result.last = it;
						result.found = true;
						break;
					}
				}
				return result;
			}

		template<typename T, typename Container>
			auto value_in( T && value, Container && container ) ->
			decltype( std::begin( container ) == std::end( container ) ) {

				return std::find_if( std::begin( container ), std::end( container ), [&]( auto const & v ) {
						return is_a( value, v );
						}) == std::end( container );
			}

		template<typename T, typename U, size_t N>
			bool value_in( T && value, U (&container)[N] ) { 
				return std::find_if( container, container + N, [&]( auto const & v ) {
						return is_a( value, v );
						}) == std::end( container );
			}

		template<typename T>
			class in_t {
				std::vector<T> container;
				public:
				in_t( std::vector<T> values ):
					container{ std::move( values ) } { }

				template<typename U>
					bool operator( )( U const & value ) const {
						return value_in( value, container );
					}
			};	// in_t

		template<typename T>
			auto in( std::vector<T> container ) {
				return in_t<T>{ std::move( container ) };
			}

		template<typename T, size_t N>
			auto in( T (&container)[N] ) {
				using value_t = daw::traits::root_type_t<T>;
				std::vector<value_t> values;
				std::copy_n( std::begin( container ), N, std::back_inserter( values ) );

				return in_t<value_t>{ std::move( values ) };
			}

		template<typename Container>
			auto in( Container container ) {
				using value_t = daw::traits::root_type_t<decltype( container[0] )>;
				std::vector<value_t> values;
				std::copy( std::begin( container ), std::end( container ), std::back_inserter( values ) );

				return in_t<value_t>{ std::move( values ) };
			}

		template<typename ForwardIterator, typename Container>
			find_result_t<ForwardIterator> until_values( ForwardIterator first, ForwardIterator last, Container && container ) {
				auto result = make_find_result( first, last );
				for( auto it = first; it != last; ++it ) {
					if( value_in( *it, container ) ) {
						result.last = it;
						result.found = true;
						return result;
					}
				}
				return result;
			}
		template<typename T>
			bool is_true( T && value, std::function<bool( daw::traits::root_type_t<T> )> predicate ) {
				return predicate( value );
			}

		template<typename T, typename Predicate, typename... Predicates>
			bool is_true( T && value, Predicate && predicate, Predicates && ... predicates ) {
				return is_true( std::forward<Predicate>(predicate), std::forward<T>(value) ) || is_true( std::forward<T>( value ), std::forward<Predicates>( predicates )... );
			}

		template<typename T, typename U, typename... Args>
			void expect( T && value, U && check, Args && ... args ) {
				if( !is_a( std::forward<T>( value ), std::forward<U>( check ), std::forward<Args>( args )... ) ) {
					throw ParserException{ };
				}
			}

		template<typename T, typename Predicate, typename... Predicates>
			void expect_true( T && value, Predicate && predicate, Predicates && ... predicates ) {
				if( !is_true( std::forward<T>( value ), std::forward<Predicate>( predicate ), std::forward<Predicates>( predicates )... ) ) {
					throw ParserException{ };
				}
			}

		template<typename T>
			constexpr bool is_alpha( T && value ) noexcept {
				using val_t = typename daw::traits::max_sizeof<T, decltype( 'a' )>::type;
				constexpr auto const a = static_cast<val_t>( 'a' );
				constexpr auto const A = static_cast<val_t>( 'A' );
				constexpr auto const z = static_cast<val_t>( 'z' );
				constexpr auto const Z = static_cast<val_t>( 'Z' );
				auto const tmp = static_cast<val_t>( value );
				return (a <= tmp && tmp <= z) || (A <= tmp && tmp <= Z);
			}

		template<typename T, typename Min, typename Max>
			constexpr bool in_range( T value, Min min_value, Max max_value ) noexcept {
				using val_t = typename daw::traits::max_sizeof<T, Min, Max>::type;
				return static_cast<val_t>(min_value) <= static_cast<val_t>(value) && static_cast<val_t>(value) <= static_cast<val_t>(max_value);
			}

		template<typename T>
			constexpr bool is_number( T const & value ) noexcept {
				return in_range( value, '0', '9' );
			}

		template<typename T>
			constexpr bool is_alphanumeric( T && value ) noexcept {
				return is_alpha( std::forward<T>( value ) ) || is_number( std::forward<T>( value ) );
			}

		template<typename T, typename U>
			constexpr void assert_not_equal( T && lhs, U && rhs ) {
				static_assert( daw::traits::is_comparable_v<T, U>, "lhs is not comparable to rhs" );
				if( lhs != rhs ) {
					throw ParserException{ };
				}
			}

		template<typename T, typename U>
			constexpr void assert_equal( T && lhs, U && rhs ) {
				static_assert( daw::traits::is_comparable_v<T, U>, "lhs is not comparable to rhs" );
				if( lhs == rhs ) {
					throw ParserException{ };
				}
			}

		template<typename Iterator>
			constexpr void assert_not_empty( Iterator first, Iterator last ) {
				if( first == last ) {
					throw ParserEmptyException{ };
				}
			}

		template<typename Iterator>
			constexpr void assert_empty( Iterator first, Iterator last ) {
				assert_equal( first, last );
			}

		template<typename ForwardIterator, typename StartFrom, typename GoUntil>
			auto from_to( ForwardIterator first, ForwardIterator last, StartFrom && start_from, GoUntil && go_until, bool throw_if_end_reached = false ) ->
			std::enable_if_t<daw::traits::is_comparable_v<decltype(*first), StartFrom> &&
			daw::traits::is_comparable_v<decltype(*first), GoUntil>, 
			find_result_t<ForwardIterator>> {

				auto start = until_value( first, last, std::forward<StartFrom>( start_from ) );
				if( !start ) {
					throw ParserException{ };
				}
				auto finish = until_value( start.last, last, std::forward<GoUntil>( go_until ) );
				if( throw_if_end_reached && !finish ) {
					throw ParserException{ };
				}
				auto result = make_find_result( start.first, finish.last );
				result.found = !result.empty( );
				return result;
			}

		template<typename ForwardIterator>
			auto from_to( ForwardIterator first, ForwardIterator last, std::function<bool(daw::traits::root_type_t<decltype(*first)>)> is_first, std::function<bool(daw::traits::root_type_t<decltype(*first)>)> is_last, bool throw_if_end_reached = false ) {
				auto start = until( first, last, is_first );
				if( !start ) {
					throw ParserException{ };
				}
				auto finish = until( start.last, last, is_last );
				if( throw_if_end_reached && !finish ) {
					throw ParserException{ };
				}
				return finish;
			}

		template<typename ForwardIterator, typename Divider, typename... Dividers>
			auto split_on( ForwardIterator first, ForwardIterator last, Divider && divider, Dividers && ... dividers ) {
				std::vector<ForwardIterator> endings;
				auto result = until_value( first, last, std::forward<Divider>(divider), std::forward<Dividers>( dividers )... );
				while( result ) {
					endings.push_back( result.last );
					result = until_value( result.last, last, std::forward<Divider>(divider), std::forward<Dividers>( dividers )... );
				}
				if( result.first != result.last ) {
					endings.push_back( result.last );
				}
				return endings;
			};

		template<typename ForwardIterator>
			auto split_if( ForwardIterator first, ForwardIterator last, std::function<bool(daw::traits::root_type_t<decltype(*first)>)> is_divider ) {
				std::vector<ForwardIterator> endings;
				auto result = until( first, last, is_divider );
				while( result ) {
					endings.push_back( result.last );
					result = until( result.last, last, is_divider );
				}
				if( result.first != result.last ) {
					endings.push_back( result.last );
				}
				return endings;
			}

		template<typename T> constexpr bool pred_true( T const & ) noexcept {
			return true;
		}

		template<typename T> constexpr bool pred_false( T const & ) noexcept {
			return false;
		}

		template<typename T>
			constexpr bool is_cr( T const & value ) noexcept {
				return is_a( value, '\n' );
			}

		template<typename T>
			constexpr bool is_unicode_whitespace( T val ) {
				switch( static_cast<uint32_t>(val) ) {
					case 0x00000009:	// CHARACTER TABULATION
					case 0x0000000A:	// LINE FEED
					case 0x0000000B:	// LINE TABULATION
					case 0x0000000C:	// FORM FEED
					case 0x0000000D:	// CARRIAGE RETURN
					case 0x00000020:	// SPACE
					case 0x00000085:	// NEXT LINE
					case 0x000000A0:	// NO-BREAK SPACE
					case 0x00001680:	// OGHAM SPACE MARK
					case 0x00002000:	// EN QUAD
					case 0x00002001:	// EM QUAD
					case 0x00002002:	// EN SPACE
					case 0x00002003:	// EM SPACE
					case 0x00002004:	// THREE-PER-EM SPACE
					case 0x00002005:	// FOUR-PER-EM SPACE
					case 0x00002006:	// SIX-PER-EM SPACE
					case 0x00002007:	// FIGURE SPACE 
					case 0x00002008:	// PUNCTUATION SPACE
					case 0x00002009:	// THIN SPACE
					case 0x0000200A:	// HAIR SPACE
					case 0x00002028:	// LINE SEPARATOR
					case 0x00002029:	// PARAGRAPH SEPARATOR
					case 0x0000202F:	// NARROW NO-BREAK SPACE 
					case 0x0000205F:	// MEDIUM MATHEMATICAL SPACE
					case 0x00003000:	// IDEOGRAPHIC SPACE
						return true;
					default:
						return false;
				}
			}

		template<typename T>
			constexpr bool not_unicode_whitespace( T val ) {
				return !is_unicode_whitespace( val );
			}

		template<typename ForwardIterator>
			auto trim_left( ForwardIterator first, ForwardIterator last ) {
				using val_t = daw::traits::root_type_t<decltype(*first)>; 
				auto start = until( first, last, &not_unicode_whitespace<val_t> );
				if( !start ) {
					return make_find_result( first, last );
				}
				return make_find_result( start.last, last );
			}

		template<typename ForwardIterator>
			auto trim_right( ForwardIterator first, ForwardIterator last ) {
				// Cannot start from right most index in unicode.  Must parse
				// left to right.  So that sucks for perf, try not to just right trim
				//
				using val_t = daw::traits::root_type_t<decltype(*first)>; 
				auto start = until( first, last, &not_unicode_whitespace<val_t> );
				if( !start ) {
					return make_find_result( first, last, false );
				}
				auto finish = until( start.last, last, &is_unicode_whitespace<val_t> );
				return make_find_result( first, finish.last, true );
			}


		template<typename ForwardIterator>
			auto trim( ForwardIterator first, ForwardIterator last ) {
				auto start = trim_left( first, last );
				auto finish = trim_right( start.first, last );
				return make_find_result( start.first, finish.last, static_cast<bool>( start ) || static_cast<bool>( finish ) );
			}

		template<typename T>
			constexpr bool is_space( T const & value ) noexcept {
				return is_a( value, ' ' );
			}

		template<typename T>
			constexpr bool not_space( T const & value ) noexcept {
				return !is_space( value );
			}

		template<typename T>
			class is_crlf {
				using value_type = daw::traits::root_type_t<T>;
				std::list<value_type> m_last_values;
				size_t m_count;

				bool match( ) const {
					if( m_last_values.size( ) != 2*m_count ) {
						return false;
					}
					bool result = true;
					for( auto it=m_last_values.begin( ); result && it != m_last_values.end( ); ++it ) {
						result = result && is_a( *(it), '\r' );
						++it;
						result = result && is_a( *(it), '\n' );
					}
					return result;
				}

				public:
				is_crlf( size_t count = 1 ):
					m_last_values{ },
					m_count{ count } { }

				bool operator( )( value_type const & v ) {
					m_last_values.push_back( std::move( v ) );
					while( m_last_values.size( ) > 4 ) {
						m_last_values.pop_front( );
					}
					return match( );
				}
			};    // is_cr

		namespace impl {
			template<typename Predicate>
				class negate_t {
					Predicate predicate;
					public:
					negate_t( Predicate pred ):
						predicate{ std::move( pred ) } { }

					template<typename... Args>
						bool operator( )( Args && ... args ) const {
							return !predicate( std::forward<Args>( args )... );
						}

					template<typename... Args>
						bool operator( )( Args && ... args ) {
							return !predicate( std::forward<Args>( args )... );
						}
				};    // negate_t
		}    // namespace impl

		template<typename Predicate>
			auto negate( Predicate predicate ) {
				return impl::negate_t<Predicate>{ std::move( predicate ) };
			}


		template<typename T>
			struct matcher_t {
				using value_t = daw::traits::root_type_t<T>;
				private:
				std::vector<value_t> m_to_match;
				public:
				matcher_t( std::vector<value_t> to_match ):
					m_to_match{ std::move( to_match ) } { }

				template<typename ForwardIterator>
					find_result_t<ForwardIterator> operator( )( ForwardIterator first, ForwardIterator last ) const {
						auto result = make_find_result( first, last );

						auto pos = std::search( result.first, result.last, m_to_match.begin( ), m_to_match.end( ), []( auto const & lhs, auto const & rhs ) {
								return is_a( lhs, rhs );
								} );

						if( pos != result.last ) {
							result.last = pos;
							result.found = true;
						}
						return result;
					}
			};		// matcher_t

		template<typename Container>
			auto matcher( Container const & container ) {
				using value_t = daw::traits::root_type_t<decltype(*container.begin( ))>;
				std::vector<value_t> values;
				std::copy( container.begin( ), container.end( ), std::back_inserter( values ) );
				return matcher_t<value_t>{ std::move( values ) };
			}

		template<typename T, size_t N>
			auto matcher( T (&container)[N] ) {
				using value_t = daw::traits::root_type_t<T>;
				std::vector<value_t> values;
				std::copy_n( container, N, std::back_inserter( values ) );
				return matcher_t<value_t>{ std::move( values ) };
			}

		///
		// Does the first range (first1, last1] start with the second range (first2, last2]
		//
		template<typename ForwardIterator1, typename ForwardIterator2, typename BinaryPredicate>
			bool starts_with( ForwardIterator1 first1, ForwardIterator1 last1, ForwardIterator2 first2, ForwardIterator2 last2, BinaryPredicate pred ) {
				while( first1 != last1 && first2 != last2 ) {
					if( !pred( *first1, *first2 ) ) {
						return false;
					}
					++first1;
					++first2;
				}
				return first2 == last2;	// end of second item
			}
	}    // namespace parser
}    // namespace daw

