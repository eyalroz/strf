//  Distributed under the Boost Software License, Version 1.0.
//  (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)

#define  _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <stdio.h>
#include <string.h>
#include <locale>
#include <codecvt>
#include <fstream>

#include <strf.hpp>
#include "loop_timer.hpp"

int main()
{
    std::u16string u16sample1(500, u'A');
    std::u16string u16sample2(500, u'\u0100');
    std::u16string u16sample3(500, u'\u0800');
    std::u16string u16sample4;
    for(int i = 0; i < 500; ++i) u16sample4.append(u"\U00010000");

    char u8dest[100000];
    constexpr std::size_t u8dest_size = sizeof(u8dest) / sizeof(u8dest[0]);
    char* u8dest_end = &u8dest[u8dest_size];

    escape(u8dest);

    std::cout << "\nUTF-16 to UTF-8\n";

    PRINT_BENCHMARK("strf::to(u8dest)(u16sample1)")
    {
        auto err = strf::to(u8dest)(strf::cv(u16sample1));
        (void)err;
        clobber();
    }
    PRINT_BENCHMARK("strf::to(u8dest)(u16sample2)")
    {
        auto err = strf::to(u8dest)(strf::cv(u16sample2));
        (void)err;
        clobber();
    }
    PRINT_BENCHMARK("strf::to(u8dest)(u16sample3)")
    {
        auto err = strf::to(u8dest)(strf::cv(u16sample3));
        (void)err;
        clobber();
    }
    PRINT_BENCHMARK("strf::to(u8dest)(u16sample4)")
    {
        auto err = strf::to(u8dest)(strf::cv(u16sample4));
        (void)err;
        clobber();
    }

#if defined(_MSC_VER)
// disable warning that std::codecvt_utf8_utf16 is deprecated
#pragma warning (disable:4996)
#endif

    std::codecvt_utf8_utf16<char16_t> codecvt;
    const char16_t* cu16next = nullptr;
    char* u8next = nullptr;

    std::cout << "\nUTF-16 to UTF-8 using std::codecvt_utf8_utf16<char16_t>\n";

    PRINT_BENCHMARK("std::codecvt / u16sample1")
    {
        std::mbstate_t mb{};
        codecvt.out
            ( mb
            , &*u16sample1.begin()
            , &*u16sample1.end()
            , cu16next
            , u8dest
            , u8dest_end
            , u8next);
        *u8next = '\0';
        clobber();
    }

    PRINT_BENCHMARK("std::codecvt / u16sample2")
    {
        std::mbstate_t mb{};
        codecvt.out
            ( mb
            , &*u16sample2.begin()
            , &*u16sample2.end()
            , cu16next
            , u8dest
            , u8dest_end
            , u8next);
        *u8next = '\0';
        clobber();
    }
    PRINT_BENCHMARK("std::codecvt / u16sample3")
    {
        std::mbstate_t mb{};
        codecvt.out
            ( mb
            , &*u16sample3.begin()
            , &*u16sample3.end()
            , cu16next
            , u8dest
            , u8dest_end
            , u8next);
        *u8next = '\0';
        clobber();
    }
    PRINT_BENCHMARK("std::codecvt / u16sample4")
    {
        std::mbstate_t mb{};
        codecvt.out
            ( mb
            , &*u16sample4.begin()
            , &*u16sample4.end()
            , cu16next
            , u8dest
            , u8dest_end
            , u8next);
        *u8next = '\0';
        clobber();
    }

}
