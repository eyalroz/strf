//  Distributed under the Boost Software License, Version 1.0.
//  (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)

#include <boost/stringify.hpp>
#include "loop_timer.hpp"

int main()
{
    namespace strf = boost::stringify;
    char dest[1000000];
    char* dest_end = dest + sizeof(dest);
    (void) dest_end;

    PRINT_BENCHMARK("strf::write(dest) (strf::join('a', 'b', 'c', 'd'))")
    {
        strf::write(dest) (strf::join('a', 'b', 'c', 'd'));
    }
    PRINT_BENCHMARK("strf::write(dest) ('a', 'b', 'c', 'd')")
    {
        strf::write(dest) ('a', 'b', 'c', 'd');
    }

    std::cout << "\n";

    PRINT_BENCHMARK("strf::write(dest) (strf::join_right(15)(\"Hello World\"))")
    {
        strf::write(dest) (strf::join_right(15)("Hello World"));
    }
    PRINT_BENCHMARK("strf::write(dest) (strf::fmt(\"Hello World\") > 15)")
    {
        strf::write(dest) (strf::fmt("Hello World") > 15);
    }
    std::cout << "\n";

    PRINT_BENCHMARK("strf::write(dest) (strf::join_right(4)(25))")
    {
        strf::write(dest) (strf::join_right(4)(25));
    }
    PRINT_BENCHMARK("strf::write(dest) (strf::dec(25) > 4)")
    {
        strf::write(dest) (strf::dec(25) > 4);
    }
    std::cout << "\n";

    return 1;
}
