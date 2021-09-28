// Copyright (c) Darrell Wright
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/beached/header_libraries
//

#pragma once

#include "daw_cxmath.h"
#include "daw_move.h"

#include <algorithm>
#include <cassert>
#include <ciso646>
#include <cstddef>
#include <cstdlib>
#include <iterator>
#include <memory>
#include <sys/mman.h>
#include <sys/types.h>
#include <type_traits>

#if defined( _WIN32 ) or defined( _WIN64 )
#include <malloc.h>
#define MALLOC_SIZE( ... ) _msize( __VA_ARGS__ )
#elif defined( __linux__ )
#include <malloc.h>
#define MALLOC_SIZE( ... ) malloc_usable_size( __VA_ARGS__ )
#else
#include <malloc/malloc.h>
#define MALLOC_SIZE( ... ) malloc_size( __VA_ARGS__ )
#endif

#if defined( __cpp_constexpr_dynamic_alloc )
#define DAW_CPP20CXDTOR constexpr
#else
#define DAW_CPP20CXDTOR
#endif

namespace daw {
	template<typename T, typename U, typename = void>
	struct has_op_minus : std::false_type {};

	template<typename T, typename U>
	struct has_op_minus<
	  T, U, std::void_t<decltype( std::declval<T>( ) - std::declval<U>( ) )>>
	  : std::true_type {};

	template<typename T, typename = void>
	struct is_iterator : std::false_type {};

	template<typename T>
	struct is_iterator<
	  T, std::void_t<decltype(
	       std::add_pointer_t<typename std::iterator_traits<T>::value_type>{ } )>>
	  : std::true_type {};

	template<typename T, typename = void>
	struct has_reallocate : std::false_type {};

	template<typename T>
	struct has_reallocate<T, std::void_t<typename T::has_realloc>>
	  : std::true_type {};

	template<typename T>
	struct MMapAlloc {
		using value_type = T;
		using pointer = T *;
		using size_type = std::size_t;
		using has_realloc = void;

	private:
		[[nodiscard]] static inline pointer allocate_raw( size_type count ) {
			T *result =
			  reinterpret_cast<T *>( ::mmap( nullptr, count, PROT_READ | PROT_WRITE,
			                                 MAP_ANONYMOUS | MAP_PRIVATE, -1, 0 ) );
			if( not result ) {
				throw std::bad_alloc( );
			}
			return result;
		}

	public:
		constexpr MMapAlloc( ) = default;

		[[nodiscard]] static inline pointer allocate( size_type count ) {
			return allocate_raw( sizeof( T ) * count );
		}

		[[nodiscard]] static inline size_type alloc_size( T * ) {
			return 0U;
		}
		// Attempt to reallocate if it can be done within the current block
		// The ptr returned should
		[[nodiscard]] static inline pointer reallocate( T *old_ptr,
		                                                size_type new_size ) {
			T *new_ptr = reinterpret_cast<T *>(
			  ::mmap( old_ptr, new_size * sizeof( T ), PROT_READ | PROT_WRITE,
			          MAP_FIXED | MAP_ANONYMOUS | MAP_PRIVATE, -1, 0 ) );
			if( new_ptr == old_ptr ) {
				return new_ptr;
			}
			if( not new_ptr ) {
				return allocate_raw( new_size * sizeof( T ) );
			}
			throw std::bad_alloc( );
		}

		static inline void deallocate( T *ptr, size_type sz ) {
			::munmap( ptr, sz );
		}
	};

	template<typename T>
	struct MallocAlloc {
		using value_type = T;
		using pointer = T *;
		using size_type = std::size_t;
		using has_realloc = void;

	private:
		[[nodiscard]] static inline pointer allocate_raw( size_type count ) {
			auto result = reinterpret_cast<T *>( std::malloc( count ) );
			if( not result ) {
				throw std::bad_alloc( );
			}
			return result;
		}

	public:
		constexpr MallocAlloc( ) = default;

		[[nodiscard]] static inline pointer allocate( size_type count ) {
			return allocate_raw( sizeof( T ) * count );
		}

		[[nodiscard]] static inline size_type alloc_size( T *ptr ) {
#if defined( MALLOC_SIZE )
			return MALLOC_SIZE( ptr );
#else
			return 0U;
#endif
		}
		// Attempt to reallocate if it can be done within the current block
		// The ptr returned should
		[[nodiscard]] static inline pointer reallocate( T *old_ptr,
		                                                size_type new_size ) {
			auto const max_sz = alloc_size( old_ptr );
			if( new_size < ( max_sz / sizeof( T ) ) ) {
				pointer new_ptr = reinterpret_cast<pointer>(
				  realloc( old_ptr, sizeof( T ) * new_size ) );
				assert( new_ptr == old_ptr );
				return new_ptr;
			}
			return allocate_raw( sizeof( T ) * new_size );
		}

		static inline void deallocate( T *ptr, size_type ) {
			std::free( ptr );
		}
	};

	template<typename T, typename... Args>
	constexpr T *construct_at( T *p, Args &&...args ) {
		if constexpr( std::is_aggregate_v<T> ) {
			return ::new( static_cast<void *>( p ) ) T{ DAW_FWD2( Args, args )... };
		} else {
			return ::new( static_cast<void *>( p ) ) T( DAW_FWD2( Args, args )... );
		}
	}

	template<typename Iterator, typename OutputIterator>
	constexpr OutputIterator move_n( Iterator first, std::size_t sz,
	                                 OutputIterator out_it ) noexcept {
		return std::uninitialized_move_n( first, sz, out_it ).second;
	}

	template<typename Iterator, typename OutputIterator>
	constexpr OutputIterator copy_n( Iterator first, std::size_t sz,
	                                 OutputIterator out_it ) noexcept {
		return std::uninitialized_copy_n( first, sz, out_it );
	}

	template<typename Allocator>
	struct AllocDeleter : Allocator {
		using pointer = typename std::allocator_traits<Allocator>::pointer;
		std::size_t alloc_size;

		explicit constexpr AllocDeleter( std::size_t sz,
		                                 Allocator a = Allocator{ } )
		  : Allocator{ a }
		  , alloc_size( sz ) {}

		constexpr void operator( )( pointer p ) const {
			this->deallocate( p, alloc_size );
		}
	};

	struct sized_for_overwrite_t {};
	inline constexpr sized_for_overwrite_t sized_for_overwrite =
	  sized_for_overwrite_t{ };

	template<typename T, typename Alloc = MMapAlloc<T>>
	struct Vector : Alloc {
		using value_type = T;
		using allocator_type = Alloc;
		using pointer = T *;
		using const_pointer = T const *;
		using reference = T &;
		using const_reference = T const &;
		using rvalue_reference = T &&;
		using iterator = T *;
		using const_iterator = T const *;
		using size_type = std::size_t;
		using difference_type = std::ptrdiff_t;

	private:
		using uptr_del = AllocDeleter<allocator_type>;
		using uptr_t = std::unique_ptr<value_type[], uptr_del>;
		uptr_t m_data = nullptr;
		size_type m_size = 0;

		uptr_t make_copy( Vector const &other ) {
			auto result = uptr_t( this->allocate( other.m_capacity ),
			                      uptr_del{ other.m_capacity } );
			copy_n( other.data( ), other.size( ), result.get( ) );
			return result;
		}

		explicit constexpr Vector( sized_for_overwrite_t, size_type sz )
		  : m_data( this->allocate( sz ), uptr_del{ sz } )
		  , m_size( sz ) {}

		constexpr size_type calc_size( size_type sz ) noexcept {
			if constexpr( has_reallocate<allocator_type>::value ) {
				auto const avail = allocator_type::alloc_size( data( ) );
				if( sz <= avail ) {
					return avail;
				}
			}
			return daw::cxmath::round_pow2( std::max( sz, capacity( ) * 2U ) );
		}

		static constexpr void relocate( pointer source, size_type sz,
		                                pointer destination ) {
			/*
			if constexpr( std::is_trivially_copyable_v<value_type> ) {
		#if DAW_HAS_BUILTIN( __builtin_memcpy )
			    (void)__builtin_memcpy( destination, source, sz * sizeof( T ) );
		#else
			    (void)std::memcpy( destination, source, sz * sizeof( T ) );
		#endif

			  } else*/
			if constexpr( std::is_nothrow_move_constructible_v<T> ) {
				(void)move_n( source, sz, destination );
			} else {
				(void)copy_n( source, sz, destination );
			}
		}

		static constexpr void overlapped_relocate( pointer source,
		                                           size_type where_idx,
		                                           size_type range_size,
		                                           size_type insert_size ) {
			assert( where_idx <= range_size );
			if( insert_size == 0 ) {
				return;
			}
			size_type const reloc_size = range_size - insert_size;
			auto dfirst =
			  static_cast<difference_type>( ( range_size + insert_size ) - 1U );
#if not defined( NDEBUG )
			auto dlast = static_cast<difference_type>( ( range_size + insert_size ) -
			                                           reloc_size );
#endif
			auto sfirst = static_cast<difference_type>( range_size - 1U );
			auto slast = static_cast<difference_type>( range_size - reloc_size );

			while( sfirst >= slast ) {
				if constexpr( std::is_nothrow_move_constructible_v<value_type> ) {
					construct_at( source + dfirst, DAW_MOVE( source[sfirst] ) );
				} else {
					construct_at( source + dfirst, source[sfirst] );
				}
				--dfirst;
				--sfirst;
			}
			assert( dfirst < dlast );
		}

		constexpr void resize_impl( size_type sz ) noexcept {
			pointer const old_ptr = data( );
			if( sz < m_size ) {
				if constexpr( std::is_trivially_destructible_v<value_type> ) {
					m_size = sz;
					return;
				} else {
					if( m_size == 0 ) {
						return;
					}
					while( m_size-- > sz ) {
						std::destroy_at( old_ptr + m_size );
					}
				}
			} else {
				if( sz >= capacity( ) ) {
					pointer const new_ptr = [&] {
						if constexpr( has_reallocate<allocator_type>::value ) {
							return this->reallocate( old_ptr, sz );
						} else {
							return this->allocate( sz );
						}
					}( );
					if( new_ptr != old_ptr ) {
						relocate( old_ptr, m_size, new_ptr );
						resize( 0 );
						m_data.reset( new_ptr );
					}
					m_data.get_deleter( ).alloc_size = sz;
				}
				m_size = sz;
			}
		}

	public:
		explicit constexpr Vector( ) = default;
		explicit constexpr Vector( allocator_type const &alloc = allocator_type{ } )
		  : allocator_type{ alloc } {}

		constexpr Vector( Vector && ) noexcept = default;
		constexpr Vector &operator=( Vector && ) noexcept = default;

		DAW_CPP20CXDTOR ~Vector( ) {
			resize( 0 );
		}

		/*
		constexpr Vector( Vector const &other )
		  : m_data( make_copy( other ) )
		  , m_size( other.m_size ) {}

		constexpr Vector &operator=( Vector const &rhs ) {
		  if( this != &rhs ) {
		    m_data.reset( make_copy( rhs ) );
		    m_size = rhs.m_size;
		  }
		  return *this;
		}
		*/

		template<typename Operation>
		explicit constexpr Vector( sized_for_overwrite_t, size_type sz,
		                           Operation op )
		  : Vector( sized_for_overwrite, sz ) {
			m_size = op( m_data.get( ), m_size );
			assert( m_size <= sz );
		}

		template<size_type N>
		explicit constexpr Vector( value_type const ( &ary )[N] )
		  : Vector( sized_for_overwrite, N ) {

			pointer const p = copy_n( ary, N, m_data.get( ) );
			m_size = static_cast<size_type>( p - m_data.get( ) );
		}

		template<size_type N>
		explicit constexpr Vector( value_type( &&ary )[N] )
		  : Vector( sized_for_overwrite, N ) {

			pointer const p = move_n( ary, N, m_data.get( ) );
			m_size = static_cast<size_type>( p - m_data.get( ) );
		}

		template<typename IteratorF, typename IteratorL,
		         std::enable_if_t<( has_op_minus<IteratorF, IteratorL>::value and
		                            is_iterator<IteratorF>::value ),
		                          std::nullptr_t> = nullptr>
		constexpr Vector( IteratorF first, IteratorL last )
		  : Vector( sized_for_overwrite, static_cast<size_type>( last - first ) ) {

			pointer const p = copy_n( first, last - first, m_data.get( ) );
			m_size = static_cast<size_type>( p - m_data.get( ) );
		}

		template<
		  typename IteratorF, typename IteratorL,
		  std::enable_if_t<( not has_op_minus<IteratorF, IteratorL>::value and
		                     is_iterator<IteratorF>::value ),
		                   std::nullptr_t> = nullptr>
		constexpr Vector( IteratorF first, IteratorL last ) {
			while( first != last ) {
				push_back( *first );
				++first;
			}
		}

		template<typename Operation>
		constexpr void resize_for_overwrite( size_type sz, Operation op ) noexcept {
			resize_impl( sz );
			m_size = op( data( ), size( ) );
		}

		[[nodiscard]] constexpr reference back( ) {
			return *( data( ) + m_size - 1 );
		}

		[[nodiscard]] constexpr const_reference back( ) const {
			return *( data( ) + m_size - 1 );
		}

		[[nodiscard]] constexpr reference front( ) {
			return *data( );
		}

		[[nodiscard]] constexpr const_reference front( ) const {
			return *data( );
		}

		constexpr void pop_back( ) {
			if( not std::is_trivially_destructible_v<value_type> ) {
				pointer ptr = std::addressof( back( ) );
				std::destroy_at( ptr );
			}
			--m_size;
		}

		constexpr void resize( size_type sz ) noexcept {
			resize_for_overwrite( sz, [&]( pointer p, size_type ) {
				if( sz < size( ) ) {
					while( size( ) > sz ) {
						pop_back( );
					}
				} else if( sz > size( ) ) {
					(void)std::fill_n( p + size( ), sz - size( ), value_type{ } );
				}
				return sz;
			} );
		}

		template<typename U>
		constexpr void resize( size_type sz, U const &value ) noexcept {
			resize_for_overwrite( sz, [&]( pointer p, size_type ) {
				if( size( ) > sz ) {
					do {
						pop_back( );
					} while( size( ) > sz );
				} else if( size( ) < sz ) {
					(void)std::fill_n( p + size( ), sz - size( ), value );
				}
			} );
		}

		[[nodiscard]] constexpr pointer data( ) {
			return m_data.get( );
		}

		[[nodiscard]] constexpr const_pointer data( ) const {
			return m_data.get( );
		}

		[[nodiscard]] constexpr size_type size( ) const {
			return m_size;
		}

		[[nodiscard]] constexpr bool empty( ) const {
			return size( ) == 0;
		}

		[[nodiscard]] constexpr size_type capacity( ) const {
			if( m_data ) {
				return m_data.get_deleter( ).alloc_size;
			}
			return 0;
		}

		constexpr void reserve( size_type n ) {
			if( n > capacity( ) ) {
				resize_impl( n );
			}
		}
		constexpr reference push_back( const_reference v ) {
			if( m_size >= capacity( ) ) {
				resize_impl( calc_size( m_size + 1 ) );
			}
			return *construct_at( m_data.get( ) + ( m_size++ ), v );
		}

		constexpr reference push_back( rvalue_reference v ) {
			if( m_size >= capacity( ) ) {
				resize_impl( calc_size( m_size + 1 ) );
			}
			return *construct_at( m_data.get( ) + ( m_size++ ), DAW_MOVE( v ) );
		}

		template<typename... Args>
		constexpr reference emplace_back( Args &&...args ) {
			if( m_size >= capacity( ) ) {
				resize_impl( calc_size( m_size + 1 ) );
			}
			return *construct_at( m_data.get( ) + ( +m_size++ ),
			                      DAW_FWD2( Args, args )... );
		}

		[[nodiscard]] iterator begin( ) {
			return iterator{ data( ) };
		}

		[[nodiscard]] const_iterator begin( ) const {
			return const_iterator{ data( ) };
		}

		[[nodiscard]] const_iterator cbegin( ) const {
			return const_iterator{ data( ) };
		}

		[[nodiscard]] iterator end( ) {
			return iterator{ data( ) + size( ) };
		}

		[[nodiscard]] const_iterator end( ) const {
			return const_iterator{ data( ) + size( ) };
		}

		[[nodiscard]] const_iterator cend( ) const {
			return const_iterator{ data( ) + size( ) };
		}

		[[nodiscard]] constexpr const_reference operator[]( size_type idx ) const {
			return data( )[idx];
		}

		[[nodiscard]] constexpr reference operator[]( size_type idx ) {
			return data( )[idx];
		}

		constexpr iterator insert( const_iterator where, const_reference value ) {
			auto const *first = &value;
			return insert( where, first, first + 1 );
		}

		template<
		  typename IteratorF, typename IteratorL,
		  std::enable_if_t<( not has_op_minus<IteratorF, IteratorL>::value and
		                     is_iterator<IteratorF>::value ),
		                   std::nullptr_t> = nullptr>
		constexpr iterator insert( const_iterator where, IteratorF first,
		                           IteratorL last ) {
			auto idx = static_cast<size_type>( where - data( ) );

			while( first != last ) {
				insert( data( )[idx], *first );
				++idx;
				++first;
			}
		}

		template<typename IteratorF, typename IteratorL,
		         std::enable_if_t<( has_op_minus<IteratorF, IteratorL>::value and
		                            is_iterator<IteratorF>::value ),
		                          std::nullptr_t> = nullptr>
		constexpr iterator insert( const_iterator where, IteratorF first,
		                           IteratorL last ) {
			size_type const old_size = size( );
			size_type const where_idx = static_cast<size_type>( where - data( ) );
			size_type const insert_size = static_cast<size_type>( last - first );
			auto const needed_size = insert_size + old_size;

			if( capacity( ) <= needed_size ) {
				resize_impl( needed_size );
			} else {
				m_size += needed_size;
			}
			if( where_idx < old_size ) {
				overlapped_relocate( data( ), where_idx, old_size, insert_size );
			}
			pointer pos = data( ) + static_cast<difference_type>( where_idx );
			while( first != last ) {
				*pos = *first;
				++pos;
				++first;
			}
			return data( ) + where_idx;
		}

		template<typename U, std::size_t N>
		iterator insert( const_iterator where, U const ( &values )[N] ) {
			return insert( where, values, values + N );
		}
	};

	template<typename T, std::size_t N>
	Vector( T const ( & )[N] ) -> Vector<T>;
} // namespace daw