// The MIT License (MIT)
//
// Copyright (c) 2014-2020 Darrell Wright
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files( the "Software" ), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and / or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include "daw/daw_benchmark.h"
#include "daw/daw_traits.h"
#include "daw/daw_utility.h"
#include "daw/daw_value_ptr.h"

#include <cstddef>
#include <iostream>
#include <mutex>
#include <type_traits>

void daw_value_ptr_test_01( ) {
	struct lrg {
		size_t a{};
		size_t b{};
		size_t c{};
		size_t d{};
		size_t e{};
	};
	std::cout << "sizeof( int ) -> " << sizeof( int ) << '\t';
	std::cout << "sizeof( daw::value_ptr<int> ) -> "
	          << sizeof( daw::value_ptr<int> ) << '\n';
	std::cout << "sizeof( size_t ) -> " << sizeof( size_t ) << '\t';
	std::cout << "sizeof( daw::value_ptr<size_t> ) -> "
	          << sizeof( daw::value_ptr<size_t> ) << '\n';
	std::cout << "sizeof( lrg ) -> " << sizeof( lrg ) << '\t';
	std::cout << "sizeof( daw::value_ptr<lrg> ) -> "
	          << sizeof( daw::value_ptr<lrg> ) << '\n';
	auto a = daw::value_ptr<int>( );
	auto b = daw::value_ptr<int>( 1 );
	auto c = daw::value_ptr<int>( 2 );
	auto d = daw::value_ptr<int>( 1 );

	// a & b
	daw::expecting( *a, 0 );
	auto test_01 = !( a == b );
	auto test_02 = !( b == a );
	auto test_03 = a != b;
	auto test_04 = b != a;
	auto test_05 = a < b;
	auto test_06 = !( b < a );
	auto test_07 = a <= b;
	auto test_08 = !( b <= a );
	auto test_09 = !( a >= b );
	auto test_10 = b >= a;
	auto test_11 = b == d;
	auto test_12 = b != c;
	auto test_13 = b < c;
	auto test_14 = b <= c;
	auto test_15 = c > b;
	auto test_16 = c >= b;

	daw::expecting( test_01 );
	daw::expecting( test_02 );
	daw::expecting( test_03 );
	daw::expecting( test_04 );
	daw::expecting( test_05 );
	daw::expecting( test_06 );
	daw::expecting( test_07 );
	daw::expecting( test_08 );
	daw::expecting( test_09 );
	daw::expecting( test_10 );
	daw::expecting( test_11 );
	daw::expecting( test_12 );
	daw::expecting( test_13 );
	daw::expecting( test_14 );
	daw::expecting( test_15 );
	daw::expecting( test_16 );

	struct A {
		constexpr A( ) noexcept {}
		A( A const & ) = delete;
		A &operator=( A const & ) = delete;
	};
	auto e = daw::value_ptr<A>( );

	auto f = std::move( e );

	auto g = daw::value_ptr<int>( );
	auto const h = g;

	g = 5;

	auto hash_value = std::hash<decltype( g )>{}( g );
	Unused( hash_value );

	auto i = daw::value_ptr<std::mutex>( );
	i.reset( );

	static_assert( daw::traits::is_regular<daw::value_ptr<int>>, "" );
	static_assert( !daw::traits::is_regular_v<daw::value_ptr<std::mutex>>, "" );

	daw::value_ptr<std::mutex> blah{};
}

struct virt_A {
	virtual ~virt_A( ) noexcept;
	virtual int operator( )( ) const {
		return 0;
	}
};

virt_A::~virt_A( ) noexcept = default;

struct virt_B : virt_A {
	int operator( )( ) const override {
		return 1;
	}
	~virt_B( ) noexcept override;
};

virt_B::~virt_B( ) noexcept = default;

void virtual_inheritance_test( ) {
	using test_t = daw::value_ptr<virt_A>;
	auto a = test_t( );
	auto b = test_t::emplace<virt_B>( );

	daw::expecting( a( ) != b( ) );
	std::cout << "a: " << a( ) << ", " << sizeof( virt_A ) << '\n';
	std::cout << "b: " << b( ) << ", " << sizeof( virt_B ) << '\n';
}

int main( ) {
	daw_value_ptr_test_01( );
	virtual_inheritance_test( );
}
