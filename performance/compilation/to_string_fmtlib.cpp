#include "fmt/format.h"
#include "args.hpp"

void FUNCTION_NAME (std::string& out)
{
    out += fmt::format
        ( "blah blah blah {:>10} {} {:>+5} blah {:>#6o} {:x} {}\n",
          arg_a0, arg_a1, arg_a2, arg_a3, arg_a4, arg_a5);

    out += fmt::format
        ( "blah blah {} {:>9}{} {} {:>+5} blah {:>#6o} {:x} {}\n",
          arg_b0, arg_b1, arg_b2, arg_b3, arg_b4, arg_b5, arg_b6, arg_b7);

    out += fmt::format
        ( "blah blah {} {:>10} {} {:>+5} {} {:>#6o} {:x} {}\n",
          arg_c0, arg_c1, arg_c2, arg_c3, arg_c4, arg_c5, arg_c6, arg_c7);
}
