//  Distributed under the Boost Software License, Version 1.0.
//  (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)

#define  _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <clocale>
#include <stdio.h>
#include <cstring>
#include <climits>
#include <boost/stringify.hpp>
#include "loop_timer.hpp"
#include "fmt/format.h"


int main()
{
    namespace strf = boost::stringify;
    char dest[1000000];
    char* dest_end = dest + sizeof(dest);
    (void) dest_end;

    std::cout << "               Without positional parameters\n";

    PRINT_BENCHMARK("strf::write(dest) .tr(\"{}\", \"Hello World!\")")
    {
        (void)strf::write(dest) .tr("{}", "Hello World!");
    }
    PRINT_BENCHMARK("fmt::format_to(dest, \"{}\", \"Hello World!\")")
    {
        fmt::format_to(dest, "{}", "Hello World!");
    }

    std::cout << "\n";

    PRINT_BENCHMARK("strf::write(dest) .tr(\"ten = {}, twenty = {}\", 10, 20)")
    {
        (void)strf::write(dest).tr("ten = {}, twenty = {}", 10, 20);
    }
    PRINT_BENCHMARK("fmt::format_to(dest, \"ten = {}, twenty = {}\", 10, 20)")
    {
        fmt::format_to(dest, "ten = {}, twenty = {}", 10, 20);
    }

    std::cout << "\n               With positional parameters\n";

    PRINT_BENCHMARK("strf::write(dest) .tr(\"ten = {1}, twenty = {0}\", 20, 10)")
    {
        (void)strf::write(dest).tr("ten = {}, twenty = {}", 10, 20);
    }
    PRINT_BENCHMARK("fmt::format_to(dest, \"ten = {1}, twenty = {0}\", 20, 10)")
    {
        fmt::format_to(dest, "ten = {}, twenty = {}", 10, 20);
    }
    return 1;
}
