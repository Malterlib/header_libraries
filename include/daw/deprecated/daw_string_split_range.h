// Copyright (c) Darrell Wright
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/beached/header_libraries
//

#pragma once

#include "daw/ciso646.h"
#include "daw/daw_algorithm.h"
#include "daw/daw_move.h"
#include "daw/daw_string_view.h"
#include "daw_string_view1.h"
#include "daw/daw_traits.h"
#include "daw/iterator/daw_reverse_iterator.h"

#include <string>

namespace daw {
	template<typename CharT>
	class string_split_iterator {
		static constexpr size_t const npos =
		  daw::sv1::basic_string_view<CharT>::npos;

		daw::sv1::basic_string_view<CharT> m_str{ };
		daw::sv1::basic_string_view<CharT> m_delemiter{ };
		size_t m_pos = npos;

		constexpr size_t find_next( ) const noexcept {
			if( m_str.empty( ) ) {
				return npos;
			}
			return m_str.find( m_delemiter, m_pos );
		}

		constexpr void move_next( ) {
			auto next_pos = find_next( );
			if( next_pos == npos ) {
				m_pos = npos;
				return;
			}
			m_pos = next_pos + m_delemiter.size( );
		}

	public:
		using iterator_category = std::forward_iterator_tag;
		using value_type = daw::sv1::basic_string_view<CharT>;
		using reference = daw::sv1::basic_string_view<CharT>;
		using pointer = daw::sv1::basic_string_view<CharT>;
		using difference_type = ptrdiff_t;

		string_split_iterator( ) = default;

		constexpr string_split_iterator(
		  daw::sv1::basic_string_view<CharT> str,
		  daw::sv1::basic_string_view<CharT> delemiter ) noexcept
		  : m_str( str )
		  , m_delemiter( delemiter )
		  , m_pos( 0 ) {}

		constexpr string_split_iterator &operator++( ) noexcept {
			move_next( );
			return *this;
		}

		constexpr string_split_iterator operator++( int ) const noexcept {
			string_split_iterator tmp{ *this };
			tmp.move_next( );
			return tmp;
		}

		constexpr daw::sv1::basic_string_view<CharT> operator*( ) const noexcept {
			auto result{ m_str };
			result.remove_prefix( m_pos );
			return result.substr( 0, find_next( ) - m_pos );
		}

		constexpr daw::sv1::basic_string_view<CharT> operator->( ) const noexcept {
			auto result{ m_str };
			result.remove_prefix( m_pos );
			return result.substr( 0, find_next( ) - m_pos );
		}

		constexpr bool equal_to( string_split_iterator const &rhs ) const noexcept {
			if( m_pos == npos and rhs.m_pos == npos ) {
				return true;
			}
			return std::tie( m_str, m_pos, m_delemiter ) ==
			       std::tie( rhs.m_str, rhs.m_pos, rhs.m_delemiter );
		}
	};

	template<typename CharT>
	string_split_iterator( daw::sv1::basic_string_view<CharT>,
	                       daw::sv1::basic_string_view<CharT> )
	  -> string_split_iterator<CharT>;

	template<typename CharT>
	constexpr bool
	operator==( string_split_iterator<CharT> const &lhs,
	            string_split_iterator<CharT> const &rhs ) noexcept {
		return lhs.equal_to( rhs );
	}

	template<typename CharT>
	constexpr bool
	operator!=( string_split_iterator<CharT> const &lhs,
	            string_split_iterator<CharT> const &rhs ) noexcept {
		return !lhs.equal_to( rhs );
	}

	template<typename CharT>
	struct string_split_range {
		using iterator = string_split_iterator<CharT>;

	private:
		iterator m_first{ };

	public:
		string_split_range( ) = default;

		constexpr string_split_range(
		  daw::sv1::basic_string_view<CharT> str,
		  daw::sv1::basic_string_view<CharT> delemiter ) noexcept
		  : m_first( str, delemiter ) {}

		constexpr iterator begin( ) const {
			return m_first;
		}

		constexpr iterator cbegin( ) const {
			return m_first;
		}

		constexpr iterator end( ) const {
			return iterator{ };
		}

		constexpr iterator cend( ) const {
			return iterator{ };
		}
	};

	template<typename CharT>
	string_split_range( daw::sv1::basic_string_view<CharT>,
	                    daw::sv1::basic_string_view<CharT> )
	  -> string_split_range<CharT>;

	template<typename CharT>
	string_split_range<CharT>
	split_string( std::basic_string<CharT> const &str,
	              daw::sv1::basic_string_view<CharT> delemiter ) noexcept {

		return { { str.data( ), str.size( ) }, delemiter };
	}

	template<typename CharT>
	string_split_range<CharT>
	split_string( std::basic_string<CharT> const &str,
	              std::basic_string<CharT> &delemiter ) noexcept {

		return { { str.data( ), str.size( ) },
		         { delemiter.data( ), delemiter.size( ) } };
	}

	template<typename CharT>
	constexpr string_split_range<CharT>
	split_string( daw::sv1::basic_string_view<CharT> str,
	              daw::sv1::basic_string_view<CharT> delemiter ) noexcept {

		return { str, delemiter };
	}

	template<typename CharT, typename Bounds, std::ptrdiff_t Ex, size_t N>
	constexpr string_split_range<CharT>
	split_string( daw::sv1::basic_string_view<CharT, Bounds, Ex> str,
	              CharT const ( &delemiter )[N] ) noexcept {

		return { str,
		         daw::sv1::basic_string_view<CharT, Bounds, N - 1>( delemiter ) };
	}

	template<typename CharT,
	         daw::sv2::string_view_bounds_type Bounds,
	         std::ptrdiff_t Ex,
	         size_t N>
	constexpr string_split_range<CharT>
	split_string( daw::sv2::basic_string_view<CharT, Bounds> str,
	              CharT const ( &delemiter )[N] ) noexcept {

		return { str, daw::sv2::basic_string_view<CharT, Bounds>( delemiter ) };
	}

	template<typename CharT,
	         typename Bounds = daw::sv1::default_string_view_bounds_type,
	         size_t N,
	         size_t M>
	constexpr string_split_range<CharT>
	split_string( CharT const ( &str )[N],
	              CharT const ( &delemiter )[M] ) noexcept {

		return { daw::sv1::basic_string_view<CharT, Bounds, N - 1>( str ),
		         daw::sv1::basic_string_view<CharT, Bounds, M - 1>( delemiter ) };
	}
} // namespace daw
