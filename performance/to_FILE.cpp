//  Distributed under the Boost Software License, Version 1.0.
//  (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)

#define  _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <climits>
#include <stdio.h>
#include <boost/stringify.hpp>
#include "loop_timer.hpp"
#include "fmt/format.h"

int main()
{
    namespace strf = boost::stringify;

#ifdef _WIN32
    FILE* dest = fopen("NUL", "w");
#else
    FILE* dest = fopen("/dev/null", "w");
#endif

    std::cout << "\n small strings \n";

    PRINT_BENCHMARK("boost::stringify::write_to(dest) = {\"Hello \", \"World\", \"!\"}")
    {
        auto err = strf::write_to(dest) = {"Hello ", "World", "!"};
        (void)err;
    }
    PRINT_BENCHMARK("boost::stringify::write_to(dest) [\"Hello {}!\"] = {\"World\"}")
    {
        auto err = strf::write_to(dest) ["Hello {}!"] = {"World"};
        (void)err;
    }
    PRINT_BENCHMARK("fmt::print(dest, \"Hello {}!\", \"World\")")
    {
        fmt::print(dest, "Hello {}!", "World");
    }
    PRINT_BENCHMARK("fprintf(dest, \"Hello %s!\", \"World\")")
    {
        fprintf(dest, "Hello %s!", "World");
    }

    std::cout << "\n long string ( 1000 characters ): \n";

    {
        std::string std_string_long_string(1000, 'x');
        const char* long_string = std_string_long_string.c_str();

        PRINT_BENCHMARK("boost::stringify::write_to(dest) = {\"Hello \", long_string, \"!\"}")
        {
            auto err = strf::write_to(dest) = {"Hello ", long_string, "!"};
            (void)err;
        }
        PRINT_BENCHMARK("boost::stringify::write_to(dest) [\"Hello {}!\"] = {long_string}")
        {
            auto err = strf::write_to(dest) ["Hello {}!"] = {long_string};
            (void)err;
        }
        PRINT_BENCHMARK("fmt::print(dest, \"Hello {}!\", long_string)")
        {
            fmt::print(dest, "Hello {}!", long_string);
        }
        PRINT_BENCHMARK("fprintf(dest, \"Hello %s!\", long_string)")
        {
            fprintf(dest, "Hello %s!", long_string);
        }
    }

    std::cout << "\n padding \n";

    PRINT_BENCHMARK("boost::stringify::write_to(dest) = {{\"aa\", 20}}")
    {
        auto err = strf::write_to(dest) = {{"aa", 20}};
        (void)err;
    }
    PRINT_BENCHMARK("boost::stringify::write_to(dest) .with(strf::width(20)) = {\"aa\"}")
    {
        auto err = strf::write_to(dest) .with(strf::width(20)) = {"aa"};
        (void)err;
    }
    PRINT_BENCHMARK("boost::stringify::write_to(dest) = { {join_right(20), {\"aa\"}} }")
    {
        auto err = strf::write_to(dest) = { {strf::join_right(20), {"aa"}} };
        (void)err;
    }
    PRINT_BENCHMARK("fmt::print(dest, \"{:20}\", \"aa\")")
    {
        fmt::print(dest, "{:20}", "aa");
    }
    PRINT_BENCHMARK("fprintf(dest, \"%20s\", \"aa\")")
    {
        fprintf(dest, "%20s", "aa");
    }

    std::cout << "\n integers \n";

    PRINT_BENCHMARK("boost::stringify::write_to(dest) = {25}")
    {
        auto err = strf::write_to(dest) = {25};
        (void)err;
    }
    PRINT_BENCHMARK("fmt::print(dest, \"{}\", 25)")
    {
        fmt::print(dest, "{}", 25);
    }
    PRINT_BENCHMARK("fprintf(dest, \"%d\", 25)")
    {
        fprintf(dest, "%d", 25);
    }

    std::cout << std::endl;
    PRINT_BENCHMARK("boost::stringify::write_to(dest) = {LLONG_MAX}")
    {
        auto err = strf::write_to(dest) = {LLONG_MAX};
        (void)err;
    }
    PRINT_BENCHMARK("fmt::print(dest, \"{}\", LLONG_MAX)")
    {
        fmt::print(dest, "{}", LLONG_MAX);
    }
    PRINT_BENCHMARK("fprintf(dest, \"%lld\", LLONG_MAX)")
    {
        fprintf(dest, "%lld", LLONG_MAX);
    }

    std::cout << std::endl;
    PRINT_BENCHMARK("boost::stringify::write_to(dest) = {LLONG_MAX, LLONG_MAX, LLONG_MAX}")
    {
        auto err = strf::write_to(dest) = {LLONG_MAX, LLONG_MAX, LLONG_MAX};
        (void)err;
    }
    PRINT_BENCHMARK("boost::stringify::write_to(dest) [\"{}{}{}\"] = {LLONG_MAX, LLONG_MAX, LLONG_MAX}")
    {
        auto err = strf::write_to(dest) ["{}{}{}"] = {LLONG_MAX, LLONG_MAX, LLONG_MAX};
        (void)err;
    }
    PRINT_BENCHMARK("fmt::print(dest, \"{}{}{}\", LLONG_MAX, LLONG_MAX, LLONG_MAX)")
    {
        fmt::print(dest, "{}{}{}", LLONG_MAX, LLONG_MAX, LLONG_MAX);
    }
    PRINT_BENCHMARK("fprintf(dest, \"%d%d%d\", LLONG_MAX, LLONG_MAX, LLONG_MAX)")
    {
        fprintf(dest, "%lld%lld%lld", LLONG_MAX, LLONG_MAX, LLONG_MAX);
    }

    std::cout << "\n formatted integers \n";

    PRINT_BENCHMARK("boost::stringify::write_to(dest) [\"{}{}{}\"] = {55555, {55555, {8, \"<+\"}} , {55555, \"#x\"}}")
    {
        auto err = strf::write_to(dest) ["{}{}{}"] = {55555, {55555, {8, "<+"}} , {55555, "#x"}};
        (void)err;
    }
    PRINT_BENCHMARK("boost::stringify::write_to(dest) = {55555, {55555, {8, \"<+\"}} , {55555, \"#x\"}}")
    {
        auto err = strf::write_to(dest) = {55555, {55555, {8, "<+"}} , {55555, "#x"}};
        (void)err;
    }
    PRINT_BENCHMARK("fmt::print(dest, \"{}{:<8}{:#x}\", 55555, 55555, 55555)")
    {
        fmt::print(dest, "{}{:<8}{:#x}", 55555, 55555, 55555);
    }
    PRINT_BENCHMARK("sprintf(dest, \"%d%-+8d%#x\", 55555, 55555, 55555)")
    {
        fprintf(dest, "%d%-+8d%#x", 55555, 55555, 55555);
    }


    std::cout << "\n Strings and itegers mixed: \n";

    PRINT_BENCHMARK("boost::stringify::write_to(dest) [\"blah blah {} blah {} blah {}\"] = {INT_MAX, {1234, {8, \"<#x\"}}, \"abcdef\"}")
    {
        auto err = strf::write_to(dest) ["blah blah {} blah {} blah {}"] = {INT_MAX, {1234, {8, "<#x"}}, "abcdef"};
        (void)err;
    }
    PRINT_BENCHMARK("fmt::print(dest, \"blah blah {} blah {:<#8x} blah {}\", INT_MAX, 1234, \"abcdef\")")
    {
        fmt::print(dest, "blah blah {} blah {:<#8x} blah {}", INT_MAX, 1234, "abcdef");
    }
    PRINT_BENCHMARK("fprintf(dest, \"blah blah %d blah %#-8x blah %s\", INT_MAX, 1234, \"abcdef\")")
    {
        fprintf(dest, "blah blah %d blah %#-8x blah %s", INT_MAX, 1234, "abcdef");
    }


    std::cout << std::endl;
    PRINT_BENCHMARK("boost::stringify::write_to(dest) [\"ten = {}, twenty = {}\"] = {10, 20}")
    {
        auto err = strf::write_to(dest) ["ten = {}, twenty = {}"] = {10, 20};
        (void)err;
    }
    PRINT_BENCHMARK("boost::stringify::write_to(dest) = {\"ten =  \", 10, \", twenty = \", 20}")
    {
        auto err = strf::write_to(dest) = {"ten =  ", 10, ", twenty = ", 20};
        (void)err;
    }
    PRINT_BENCHMARK("fmt::print(dest, \"ten = {}, twenty = {}\", 10, 20)")
    {
        fmt::print(dest, "ten = {}, twenty = {}", 10, 20);
    }
    PRINT_BENCHMARK("fprintf(dest, \"ten = %d, twenty= %d\", 10, 20)")
    {
        fprintf(dest, "ten = %d, twenty= %d", 10, 20);
    }


    fclose(dest);
    return 1;
}