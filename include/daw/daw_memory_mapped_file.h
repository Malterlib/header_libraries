// Copyright (c) Darrell Wright
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/beached/header_libraries
//

#pragma once

#include "daw_string_view.h"
#include "daw_traits.h"

#include <ciso646>
#include <cstddef>
#include <cstdio>
#include <string_view>
#include <utility>

#if not defined( _MSC_VER )
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <type_traits>
#include <unistd.h>
#else
#include <algorithm>
#include <cstdio>
#include <string>
#include <tchar.h>
#include <windows.h>
#endif

namespace daw::filesystem {
	enum class open_mode : bool { read, read_write };

#if not defined( _MSC_VER )
	/***
	 * A RAII Memory Mapped File object
	 */
	template<typename T = char>
	struct memory_mapped_file_t {
		using value_type = T;
		using reference = T &;
		using const_reference =
		  std::conditional_t<std::is_const_v<T>, T, T const> &;
		using pointer = T *;
		using const_pointer = std::conditional_t<std::is_const_v<T>, T, T const> *;
		using size_type = size_t;

	private:
		int m_file = -1;
		pointer m_ptr = nullptr;
		size_type m_size = 0;

		void cleanup( ) noexcept {
			if( m_ptr != nullptr ) {
				munmap( m_ptr, m_size );
				m_ptr = nullptr;
			}
			m_size = 0;
			if( m_file >= 0 ) {
				close( m_file );
				m_file = -1;
			}
		}

	public:
		constexpr explicit memory_mapped_file_t( ) noexcept = default;

		/***
		 * Open memory mapped file with file path supplied
		 * Caller is responsible for ensuring that the StringView is zero terminated
		 */
		template<typename StringView,
		         std::enable_if_t<daw::traits::is_string_view_like_v<StringView>,
		                          std::nullptr_t> = nullptr>
		explicit memory_mapped_file_t( StringView file,
		                               open_mode mode = open_mode::read ) noexcept {

			(void)open( std::string_view( std::data( file ), std::size( file ) ),
			            mode );
		}

		/***
		 * Open memory mapped file with file path supplied
		 * Caller is responsible for ensuring that the StringView is zero terminated
		 */
		explicit memory_mapped_file_t( char const *file,
		                               open_mode mode = open_mode::read ) noexcept {

			(void)open( std::string_view( file ), mode );
		}

		/***
		 * Open memory mapped file with file path supplied
		 * Caller is responsible for ensuring that the StringView is zero terminated
		 */
		[[nodiscard]] bool open( std::string_view file,
		                         open_mode mode = open_mode::read ) noexcept {

			m_file =
			  ::open( file.data( ), mode == open_mode::read ? O_RDONLY : O_RDWR );
			if( m_file < 0 ) {
				return false;
			}
			{
				auto const fsz = lseek( m_file, 0, SEEK_END );
				lseek( m_file, 0, SEEK_SET );
				if( fsz <= 0 ) {
					cleanup( );
					return false;
				}
				m_size = static_cast<size_type>( fsz );
			}
			m_ptr = static_cast<pointer>(
			  mmap( nullptr, m_size,
			        mode == open_mode::read ? PROT_READ : PROT_READ | PROT_WRITE,
			        MAP_SHARED, m_file, 0 ) );

			if( m_ptr == MAP_FAILED ) {
				m_ptr = nullptr;
				cleanup( );
				return false;
			}
			return true;
		}

		[[nodiscard]] reference operator[]( size_type pos ) noexcept {
			return m_ptr[pos];
		}

		[[nodiscard]] const_reference operator[]( size_t pos ) const noexcept {
			return m_ptr[pos];
		}

		[[nodiscard]] constexpr pointer data( ) noexcept {
			return m_ptr;
		}

		[[nodiscard]] constexpr const_pointer data( ) const noexcept {
			return m_ptr;
		}

		[[nodiscard]] constexpr pointer begin( ) noexcept {
			return m_ptr;
		}

		[[nodiscard]] constexpr const_pointer begin( ) const noexcept {
			return m_ptr;
		}

		[[nodiscard]] constexpr const_pointer cbegin( ) const noexcept {
			return m_ptr;
		}

		[[nodiscard]] constexpr pointer end( ) noexcept {
			return m_ptr + static_cast<ptrdiff_t>( m_size );
		}

		[[nodiscard]] constexpr const_pointer end( ) const noexcept {
			return m_ptr + static_cast<ptrdiff_t>( m_size );
		}

		[[nodiscard]] constexpr const_pointer cend( ) const noexcept {
			return m_ptr + static_cast<ptrdiff_t>( m_size );
		}

		[[nodiscard]] constexpr size_type size( ) const noexcept {
			return m_size;
		}

		constexpr explicit operator bool( ) const noexcept {
			return m_file >= 0 and m_ptr != nullptr;
		}

		memory_mapped_file_t( memory_mapped_file_t const & ) = delete;
		memory_mapped_file_t &operator=( memory_mapped_file_t const & ) = delete;

		memory_mapped_file_t( memory_mapped_file_t &&other ) noexcept
		  : m_file( std::exchange( other.m_file, -1 ) )
		  , m_ptr( std::exchange( other.m_ptr, nullptr ) )
		  , m_size( std::exchange( other.m_size, 0 ) ) {}

		memory_mapped_file_t &operator=( memory_mapped_file_t &&rhs ) noexcept {
			if( this != &rhs ) {
				m_file = std::exchange( rhs.m_file, -1 );
				m_ptr = std::exchange( rhs.m_ptr, nullptr );
				m_size = std::exchange( rhs.m_size, 0 );
			}
			return *this;
		}

		~memory_mapped_file_t( ) noexcept {
			cleanup( );
		}

		template<typename Traits>
		operator std::basic_string_view<T, Traits>( ) const {
			return { data( ), size( ) };
		}
	};
#else
	namespace mapfile_impl {
		static constexpr long CreateFileMode( open_mode m ) {
			if( m == open_mode::read ) {
				return GENERIC_READ;
			}
			return GENERIC_READ | GENERIC_WRITE;
		}

		static constexpr long PageMode( open_mode m ) {
			if( m == open_mode::read ) {
				return PAGE_READONLY;
			}
			return PAGE_READWRITE;
		}

		static constexpr long MapMode( open_mode m ) {
			if( m == open_mode::read ) {
				return FILE_MAP_READ;
			}
			return FILE_MAP_WRITE;
		}
	} // namespace mapfile_impl

	/***
	 * A RAII Memory Mapped File object
	 */
	template<typename T = char>
	struct memory_mapped_file_t {
		using value_type = T;
		using reference = T &;
		using const_reference =
		  typename std::conditional<std::is_const<T>::value, T, T const>::type &;
		using pointer = T *;
		using const_pointer =
		  typename std::conditional<std::is_const<T>::value, T, T const>::type *;
		using size_type = size_t;

	private:
		HANDLE m_handle = nullptr;
		size_t m_size = 0;
		pointer m_ptr = nullptr;

		void cleanup( ) noexcept {
			m_size = 0;
			if( auto tmp = std::exchange( m_ptr, nullptr ); tmp ) {
				::UnmapViewOfFile( static_cast<LPVOID>( tmp ) );
			}
			if( auto tmp = std::exchange( m_handle, nullptr ); tmp ) {
				::CloseHandle( m_handle );
			}
		}

	public:
		constexpr memory_mapped_file_t( ) noexcept = default;

		/***
		 * Open memory mapped file with file path supplied
		 * Caller is responsible for ensuring that the StringView is zero terminated
		 */
		memory_mapped_file_t( std::string_view file,
		                      open_mode mode = open_mode::read ) noexcept {

			(void)open( file, mode );
		}

		/***
		 * Open memory mapped file with file path supplied
		 * Caller is responsible for ensuring that the StringView is zero terminated
		 */
		memory_mapped_file_t( std::wstring_view file,
		                      open_mode mode = open_mode::read ) noexcept {

			(void)open( file, mode );
		}

		/***
		 * Open memory mapped file with file path supplied
		 * Caller is responsible for ensuring that the StringView is zero terminated
		 */
		[[nodiscard]] bool open( std::string_view file,
		                         open_mode mode = open_mode::read ) noexcept {

			{
				HANDLE file_handle = ::CreateFileA(
				  file.data( ), mapfile_impl::CreateFileMode( mode ), 0, nullptr,
				  OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr );
				if( file_handle == INVALID_HANDLE_VALUE ) {
					return false;
				}
				LARGE_INTEGER fsz;
				if( not ::GetFileSizeEx( file_handle, &fsz ) or fsz.QuadPart <= 0 ) {
					cleanup( );
					return false;
				}
				m_size = static_cast<size_t>( fsz.QuadPart );
				m_handle = ::CreateFileMapping(
				  file_handle, nullptr, mapfile_impl::PageMode( mode ), fsz.u.HighPart,
				  fsz.u.LowPart, nullptr );
				if( m_handle == nullptr ) {
					cleanup( );
					return false;
				}
				CloseHandle( file_handle );
			}
			auto ptr =
			  MapViewOfFile( m_handle, mapfile_impl::MapMode( mode ), 0, 0, 0 );
			if( ptr == nullptr ) {
				cleanup( );
				return false;
			}
			m_ptr = static_cast<pointer>( ptr );
			return true;
		}

		[[nodiscard]] bool open( std::wstring_view file,
		                         open_mode mode = open_mode::read ) noexcept {

			{
				HANDLE file_handle = ::CreateFileW(
				  file.data( ), mapfile_impl::CreateFileMode( mode ), 0, nullptr,
				  OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr );
				if( file_handle == INVALID_HANDLE_VALUE ) {
					return false;
				}
				LARGE_INTEGER fsz;
				if( not ::GetFileSizeEx( file_handle, &fsz ) or fsz.QuadPart <= 0 ) {
					cleanup( );
					return false;
				}
				m_size = static_cast<size_t>( fsz.QuadPart );
				m_handle = ::CreateFileMapping(
				  file_handle, nullptr, mapfile_impl::PageMode( mode ), fsz.u.HighPart,
				  fsz.u.LowPart, nullptr );
				if( m_handle == nullptr ) {
					cleanup( );
					return false;
				}
				CloseHandle( file_handle );
			}
			auto ptr =
			  MapViewOfFile( m_handle, mapfile_impl::MapMode( mode ), 0, 0, 0 );
			if( ptr == nullptr ) {
				cleanup( );
				return false;
			}
			m_ptr = static_cast<pointer>( ptr );
			return true;
		}

		[[nodiscard]] reference operator[]( size_type pos ) noexcept {
			return m_ptr[pos];
		}

		[[nodiscard]] const_reference operator[]( size_t pos ) const noexcept {
			return m_ptr[pos];
		}

		[[nodiscard]] constexpr pointer data( ) noexcept {
			return m_ptr;
		}

		[[nodiscard]] constexpr const_pointer data( ) const noexcept {
			return m_ptr;
		}

		[[nodiscard]] constexpr size_type size( ) const noexcept {
			return m_size;
		}

		constexpr explicit operator bool( ) const noexcept {
			return m_size == 0 or m_ptr == nullptr or m_handle == nullptr;
		}

		memory_mapped_file_t( memory_mapped_file_t const & ) = delete;

		memory_mapped_file_t &operator=( memory_mapped_file_t const & ) = delete;

		memory_mapped_file_t( memory_mapped_file_t &&other ) noexcept
		  : m_handle( std::exchange( other.m_handle, nullptr ) )
		  , m_size( std::exchange( other.m_size, 0 ) )
		  , m_ptr( std::exchange( other.m_ptr, nullptr ) ) {}

		memory_mapped_file_t &operator=( memory_mapped_file_t &&rhs ) noexcept {
			if( this != &rhs ) {
				m_handle = std::exchange( rhs.m_handle, nullptr );
				m_size = std::exchange( rhs.m_size, 0 );
				m_ptr = std::exchange( rhs.m_ptr, nullptr );
			}
			return *this;
		}

		~memory_mapped_file_t( ) noexcept {
			cleanup( );
		}

		operator std::basic_string_view<T>( ) const {
			return { data( ), size( ) };
		}

		operator std::basic_string_view<T>( ) {
			return { data( ), size( ) };
		}
	};
#endif
} // namespace daw::filesystem
