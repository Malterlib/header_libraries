// The MIT License (MIT)
//
// Copyright (c) 2016-2017 Darrell Wright
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

#include <tuple>
#include <type_traits>

namespace daw {
	namespace impl {
		template<typename Function>
		class not_fn_t {
			Function m_function;

		  public:
			not_fn_t( ) = default;
			explicit not_fn_t( Function func ) : m_function{std::move( func )} {}
			~not_fn_t( );
			not_fn_t( not_fn_t const & ) = default;
			not_fn_t( not_fn_t && ) = default;
			not_fn_t &operator=( not_fn_t const & ) = default;
			not_fn_t &operator=( not_fn_t && ) = default;

			template<typename... Args>
			auto operator( )( Args &&... args ) {
				return !m_function( std::forward<Args>( args )... );
			}
		};

		template<typename Function>
		not_fn_t<Function>::~not_fn_t( ) {}

	} // namespace impl

	template<typename Function>
	impl::not_fn_t<Function> not_fn( Function func ) {
		return impl::not_fn_t<Function>{std::move( func )};
	}

	template<typename Function>
	impl::not_fn_t<Function> not_fn( ) {
		return impl::not_fn_t<Function>{};
	}

	template<bool B>
	using bool_constant = std::integral_constant<bool, B>;

	template<bool B>
	constexpr bool bool_consant_v = bool_constant<B>::value;

	template<typename T>
	constexpr bool is_function_v = std::is_function<T>::value;

	template<typename Base, typename Derived>
	constexpr bool is_base_of_v = std::is_base_of<Base, Derived>::value;

	template<typename T>
	constexpr bool is_member_pointer_v = std::is_member_pointer<T>::value;

	template<typename T>
	constexpr std::size_t tuple_size_v = std::tuple_size<T>::value;

	namespace detail {
		template<typename T>
		struct is_reference_wrapper : std::false_type {};

		template<typename U>
		struct is_reference_wrapper<std::reference_wrapper<U>> : std::true_type {};

		template<typename T>
		constexpr bool is_reference_wrapper_v = is_reference_wrapper<T>::value;

		template<typename Base, typename T, typename Derived, typename... Args>
		auto INVOKE( T Base::*pmf, Derived &&ref, Args &&... args ) noexcept(
		    noexcept( ( std::forward<Derived>( ref ).*pmf )( std::forward<Args>( args )... ) ) )
		    -> std::enable_if_t<daw::is_function_v<T> && daw::is_base_of_v<Base, std::decay_t<Derived>>,
		                        decltype( ( std::forward<Derived>( ref ).*pmf )( std::forward<Args>( args )... ) )> {
			return ( std::forward<Derived>( ref ).*pmf )( std::forward<Args>( args )... );
		}

		template<typename Base, typename T, typename RefWrap, typename... Args>
		auto INVOKE( T Base::*pmf, RefWrap &&ref,
		             Args &&... args ) noexcept( noexcept( ( ref.get( ).*pmf )( std::forward<Args>( args )... ) ) )
		    -> std::enable_if_t<daw::is_function_v<T> && is_reference_wrapper_v<std::decay_t<RefWrap>>,
		                        decltype( ( ref.get( ).*pmf )( std::forward<Args>( args )... ) )> {

			return ( ref.get( ).*pmf )( std::forward<Args>( args )... );
		}

		template<typename Base, typename T, typename Pointer, typename... Args>
		auto INVOKE( T Base::*pmf, Pointer &&ptr, Args &&... args ) noexcept(
		    noexcept( ( ( *std::forward<Pointer>( ptr ) ).*pmf )( std::forward<Args>( args )... ) ) )
		    -> std::enable_if_t<daw::is_function_v<T> && !is_reference_wrapper_v<std::decay_t<Pointer>> &&
		                            !daw::is_base_of_v<Base, std::decay_t<Pointer>>,
		                        decltype( ( ( *std::forward<Pointer>( ptr ) ).*
		                                    pmf )( std::forward<Args>( args )... ) )> {

			return ( ( *std::forward<Pointer>( ptr ) ).*pmf )( std::forward<Args>( args )... );
		}

		template<typename Base, typename T, typename Derived>
		auto INVOKE( T Base::*pmd, Derived &&ref ) noexcept( noexcept( std::forward<Derived>( ref ).*pmd ) )
		    -> std::enable_if_t<!daw::is_function_v<T> && daw::is_base_of_v<Base, std::decay_t<Derived>>,
		                        decltype( std::forward<Derived>( ref ).*pmd )> {

			return std::forward<Derived>( ref ).*pmd;
		}

		template<typename Base, typename T, typename RefWrap>
		auto INVOKE( T Base::*pmd, RefWrap &&ref ) noexcept( noexcept( ref.get( ).*pmd ) )
		    -> std::enable_if_t<!daw::is_function_v<T> && is_reference_wrapper_v<std::decay_t<RefWrap>>,
		                        decltype( ref.get( ).*pmd )> {
			return ref.get( ).*pmd;
		}

		template<typename Base, typename T, typename Pointer>
		auto INVOKE( T Base::*pmd, Pointer &&ptr ) noexcept( noexcept( ( *std::forward<Pointer>( ptr ) ).*pmd ) )
		    -> std::enable_if_t<!daw::is_function_v<T> && !is_reference_wrapper_v<std::decay_t<Pointer>> &&
		                            !daw::is_base_of_v<Base, std::decay_t<Pointer>>,
		                        decltype( ( *std::forward<Pointer>( ptr ) ).*pmd )> {
			return ( *std::forward<Pointer>( ptr ) ).*pmd;
		}

		template<typename F, typename... Args>
		auto INVOKE( F &&f,
		             Args &&... args ) noexcept( noexcept( std::forward<F>( f )( std::forward<Args>( args )... ) ) )
		    -> std::enable_if_t<!daw::is_member_pointer_v<std::decay_t<F>>,
		                        decltype( std::forward<F>( f )( std::forward<Args>( args )... ) )> {

			return std::forward<F>( f )( std::forward<Args>( args )... );
		}
	} // namespace detail

	template<typename F, typename... ArgTypes>
	auto invoke( F &&f, ArgTypes &&... args )
	    // exception specification for QoI
	    noexcept( noexcept( detail::INVOKE( std::forward<F>( f ), std::forward<ArgTypes>( args )... ) ) )
	        -> decltype( detail::INVOKE( std::forward<F>( f ), std::forward<ArgTypes>( args )... ) ) {

		return detail::INVOKE( std::forward<F>( f ), std::forward<ArgTypes>( args )... );
	}

	namespace detail {
		template<typename F, typename Tuple, std::size_t... I>
		constexpr decltype( auto ) apply_impl( F &&f, Tuple &&t, std::index_sequence<I...> ) {
			return daw::invoke( std::forward<F>( f ), std::get<I>( std::forward<Tuple>( t ) )... );
		}
	} // namespace detail

	template<typename F, typename Tuple>
	constexpr decltype( auto ) apply( F &&f, Tuple &&t ) {
		return detail::apply_impl( std::forward<F>( f ), std::forward<Tuple>( t ),
		                           std::make_index_sequence<daw::tuple_size_v<std::decay_t<Tuple>>>{} );
	}

	namespace detail {
		template<typename T>
		using always_void = void;

		template<typename Expr, typename Enable = void>
		struct is_callable_impl : std::false_type {};

		template<typename F, typename... Args>
		struct is_callable_impl<F( Args... ), always_void<std::result_of_t<F( Args... )>>> : std::true_type {};
	} // namespace detail

	template<typename...>
	struct is_callable : std::false_type {};

	template<typename Expr>
	struct is_callable<Expr> : detail::is_callable_impl<Expr> {};

	template<typename Function, typename... Args>
	struct is_callable<Function, Args...> : detail::is_callable_impl<Function( Args... )> {};

	template<typename... Ts>
	using is_callable_t = typename is_callable<Ts...>::type;

	template<typename... Ts>
	constexpr bool is_callable_v = is_callable<Ts...>::value;

	template<class T>
	constexpr std::add_const_t<T> &as_const( T &t ) noexcept {
		return t;
	}
} // namespace daw
