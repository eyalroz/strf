//  Distributed under the Boost Software License, Version 1.0.
//  (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)

#include "test_utils.hpp"
#include <strf.hpp>

#if ! defined(__cpp_char8_t)
using char8_t = char;
#endif

int main()
{
    {
        TEST("   abc")   ( strf::right("abc", 6) );
        TEST("abc...")   ( strf::left    ("abc", 6, '.') );
        TEST("...abc")   ( strf::right   ("abc", 6, '.') );
        TEST(".abc..")   ( strf::center  ("abc", 6, '.') );
        TEST("     abc")   ( strf::join_right(8)("abc") );
        TEST("...abc~~")   ( strf::join_right(8, '.')(strf::left("abc", 5, U'~')) );
        TEST(".....abc")   ( strf::join_right(8, '.')(strf::left("abc", 3, U'~')) );
        TEST(".....abc")   ( strf::join_right(8, '.')(strf::left("abc", 2, U'~')) );

        TEST("   abcdefghi") ( strf::right("", 3), strf::right("abc", 3), strf::left("def", 3), strf::center("ghi", 3) );
        TEST("  abcdefghi")  ( strf::right("", 2), strf::right("abc", 2), strf::left("def", 2), strf::center("ghi", 2) );
        TEST("abcdefghi")    ( strf::right("", 0), strf::right("abc", 0), strf::left("def", 0), strf::center("ghi", 0) );
    }
    {
        wchar_t abc[] = L"abc";
        wchar_t def[] = L"def";
        wchar_t ghi[] = L"ghi";
        TEST(L"abc")      ( abc );
        TEST(L"   abc")   ( strf::right(abc, 6) );
        TEST(L"abc...")   ( strf::left    (abc, 6, '.') );
        TEST(L"...abc")   ( strf::right   (abc, 6, '.') );
        TEST(L".abc..")   ( strf::center  (abc, 6, '.') );
        TEST(L"     abc")   ( strf::join_right(8)(abc) );
        TEST(L"...abc~~")   ( strf::join_right(8, '.')(strf::left(abc, 5, U'~')) );
        TEST(L".....abc")   ( strf::join_right(8, '.')(strf::left(abc, 3, U'~')) );
        TEST(L".....abc")   ( strf::join_right(8, '.')(strf::left(abc, 2, U'~')) );

        TEST(L"   abcdefghi") ( strf::right(L"", 3), strf::right(abc, 3), strf::left(def, 3), strf::center(ghi, 3) );
        TEST(L"  abcdefghi")  ( strf::right(L"", 2), strf::right(abc, 2), strf::left(def, 2), strf::center(ghi, 2) );
        TEST(L"abcdefghi")    ( strf::right(L"", 0), strf::right(abc, 0), strf::left(def, 0), strf::center(ghi, 0) );
    }


    {
        std::string abc{ "abc" };

        TEST("abc")     ( abc );
        TEST("   abc")  ( strf::right(abc, 6) );
    }

    {
        std::wstring abc{ L"abc" };

        TEST(L"abc")     ( abc );
        TEST(L"   abc")  ( strf::right(abc, 6) );
    }

#if defined(STRF_HAS_STD_STRING_VIEW)

    {
        std::string_view abc{"abcdef", 3};

        TEST("abc")    ( abc );
        TEST("   abc") ( strf::right(abc, 6) );
    }

#endif

    {   // by-pass encoding sanitization

        TEST("---\x99---") (strf::cv("---\x99---"));
        TEST("---\xA5---")
            .with(strf::iso_8859_3<char>())
            (strf::cv("---\xA5---", strf::iso_8859_3<char>()));
        TEST(reinterpret_cast<const char8_t*>("---\xA5---"))
            .with(strf::iso_8859_3<char8_t>())
            (strf::cv("---\xA5---", strf::iso_8859_3<char>()));
        TEST("...---\xA5---")
            .with(strf::iso_8859_3<char>())
            (strf::right("---\xA5---", 10, U'.').cv(strf::iso_8859_3<char>()));
        TEST(reinterpret_cast<const char8_t*>("...---\xA5---"))
            .with(strf::iso_8859_3<char8_t>())
            (strf::right("---\xA5---", 10, U'.').cv(strf::iso_8859_3<char>()));
    }
    {   // encoding sanitization

        TEST("---\xEF\xBF\xBD---") (strf::sani("---\xFF---"));
        TEST("   ---\xEF\xBF\xBD---") (strf::sani("---\xFF---") > 10);
        TEST("---\xEF\xBF\xBD---") (strf::sani("---\xFF---", strf::utf8<char>()));
        TEST("   ---\xEF\xBF\xBD---") (strf::sani("---\xFF---", strf::utf8<char>()) > 10);
        TEST("---?---")
            .with(strf::iso_8859_3<char>())
            (strf::sani("---\xA5---", strf::iso_8859_3<char>()));
        TEST("  ---?---")
            .with(strf::iso_8859_3<char>())
            (strf::sani("---\xA5---", strf::iso_8859_3<char>()) > 9);

        TEST("...---\x99---") (strf::cv("---\x99---").fill(U'.') > 10);
        TEST("...---\x99---") (strf::cv("---\x99---", strf::utf8<char>()).fill(U'.') > 10);
    }
    {   // encoding conversion

        TEST("--?--\x80--")
            .with(strf::windows_1252<char>())
            (strf::sani("--\xC9\x90--\xE2\x82\xAC--", strf::utf8<char>()));

        TEST("--?--\x80--")
            .with(strf::windows_1252<char>())
            (strf::cv("--\xC9\x90--\xE2\x82\xAC--", strf::utf8<char>()));

        TEST("....--?--\x80--")
            .with(strf::windows_1252<char>())
            (strf::right("--\xC9\x90--\xE2\x82\xAC--", 12, U'.').cv(strf::utf8<char>()));
    }

    {   // convertion from utf32

        TEST(u8"--\u0080--\u07ff--\u0800--\uffff--\U00010000--\U0010ffff")
            ( strf::cv(U"--\u0080--\u07ff--\u0800--\uffff--\U00010000--\U0010ffff") );

        TEST(u"--\u0080--\u07ff--\u0800--\uffff--\U00010000--\U0010ffff")
            ( strf::cv(U"--\u0080--\u07ff--\u0800--\uffff--\U00010000--\U0010ffff") );

        char32_t abc[] = U"abc";
        char32_t def[] = U"def";
        char32_t ghi[] = U"ghi";
        TEST("abc")      ( strf::cv(abc) );
        TEST("   abc")   ( strf::cv(abc) > 6 );
        TEST("abc...")   ( strf::cv(abc).fill('.') < 6 );
        TEST("...abc")   ( strf::cv(abc).fill('.') > 6 );
        TEST(".abc..")   ( strf::cv(abc).fill('.') ^ 6 );
        TEST("     abc")   ( strf::join_right(8)(strf::cv(abc)) );
        TEST("...abc~~")   ( strf::join_right(8, '.')(strf::cv(abc).fill(U'~') < 5));
        TEST(".....abc")   ( strf::join_right(8, '.')(strf::cv(abc).fill(U'~') < 3));
        TEST(".....abc")   ( strf::join_right(8, '.')(strf::cv(abc).fill(U'~') < 2));

        TEST("   abcdefghi") ( strf::cv(U"") > 3, strf::cv(abc)>3, strf::cv(def)<3, strf::cv(ghi)^3 );
        TEST("  abcdefghi")  ( strf::cv(U"") > 2, strf::cv(abc)>2, strf::cv(def)<2, strf::cv(ghi)^2 );
        TEST("abcdefghi")    ( strf::cv(U"") > 0, strf::cv(abc)>0, strf::cv(def)<0, strf::cv(ghi)^0 );

    }

    return test_finish();
}





