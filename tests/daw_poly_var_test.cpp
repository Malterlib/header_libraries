// The MIT License (MIT)
//
// Copyright (c) 2019-2020 Darrell Wright
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
#include "daw/daw_poly_var.h"

#include <iostream>

namespace {
	struct Base {
		Base( ) = default;
		virtual ~Base( ) = default;
		Base( Base const & ) = default;
		Base( Base && ) = default;
		Base &operator=( Base const & ) = default;
		[[maybe_unused]] Base &operator=( Base && ) = default;

		virtual int get_num( ) = 0;
	};

	struct C1 : Base {
		C1( ) = default;
		int get_num( ) override {
			return 1;
		}
	};

	struct C2 : Base {
		C2( ) = default;
		int get_num( ) override {
			return 2;
		}
	};
} // namespace

int main( int argc, char ** ) {
	if( argc % 2 == 0 ) {
		daw::poly_var<Base, C1, C2> val( C2{} );
		std::cout << "result: " << val->get_num( ) << '\n';
		return 0;
	}
	daw::poly_var<Base, C1, C2> val( C1{} );
	std::cout << "result: " << val->get_num( ) << '\n';
	return 0;
}
