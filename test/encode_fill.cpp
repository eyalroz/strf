//  Distributed under the Boost Software License, Version 1.0.
//  (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)

#include "lightweight_test_label.hpp"
#include <boost/stringify.hpp>
#include <vector>

namespace strf = boost::stringify::v0;

template <typename CharT>
std::basic_string<CharT> repeat
    ( std::size_t count
    , std::basic_string<CharT> str )
{
    std::basic_string<CharT> x;
    x.reserve(count * str.size());
    while (count--)
    {
        x.append(str);
    }
    return std::move(x);
}

template <typename CharT>
inline std::basic_string<CharT> repeat(std::size_t count, const CharT* str)
{
    return repeat<CharT>(count, std::basic_string<CharT>{str});
}

template <typename CharT>
void test_fill
    ( strf::encoding<CharT> enc
    , char32_t fill_char
    , std::basic_string<CharT> encoded_char
    , bool allow_surrogates = false )
{
    BOOST_TEST_LABEL << "test_fill_char: U+"
                     << std::hex << (unsigned)fill_char
                     << std::dec;

    strf::encoding_policy epoli
        { strf::error_handling::replace
        , allow_surrogates };

    {
        std::size_t count = 10;
        auto result = strf::to_basic_string<CharT>
            .facets(enc, epoli)
            (strf::right(CharT('x'), count + 1, fill_char));
        result.pop_back(); // remove the 'x'
        BOOST_TEST(result == repeat(count, encoded_char));
    }
    {
        std::size_t count = 200;
        auto result = strf::to_basic_string<CharT>
            .facets(enc, epoli)
            (strf::right(CharT('x'), count + 1, fill_char));
        result.pop_back(); // remove the 'x'
        BOOST_TEST(result == repeat(count, encoded_char));
    }
}

template <typename CharT>
inline void test_fill
    ( strf::encoding<CharT> enc
    , char32_t fill_char
    , const CharT* encoded_char
    , bool allow_surrogates = false )
{
    return test_fill
        ( enc
        , fill_char
        , std::basic_string<CharT>{encoded_char}
        , allow_surrogates );
}


template <typename CharT>
void test_invalid_fill_stop
    ( strf::encoding<CharT> enc
    , char32_t fill_char
    , bool allow_surrogates = false )
{
    BOOST_TEST_LABEL << "encoding: " << enc.name()
                     << "; test_fill_char: \\u'"
                     << std::hex << (unsigned)fill_char << '\''
                     << std::dec;

    strf::encoding_policy epoli
        { strf::error_handling::stop
        , allow_surrogates };

    std::basic_string<CharT> expected(5, CharT('-'));

    {
        std::size_t count = 10;
        std::basic_string<CharT> result;
        auto ec = strf::ec_assign(result)
            .facets(enc, epoli)
            ( strf::multi(CharT('-'), 5)
            , strf::right(CharT('x'), count + 1, fill_char)
            , strf::multi(CharT('+'), 5) );
        BOOST_TEST(result == expected);
        BOOST_TEST(ec == std::errc::illegal_byte_sequence);
    }
}

template <typename CharT>
void test_invalid_fill_ignore
    ( strf::encoding<CharT> enc
    , char32_t fill_char
    , bool allow_surrogates = false )
{
    BOOST_TEST_LABEL << "test_fill_char: \\u'"
                     << std::hex << (unsigned)fill_char << '\''
                     << std::dec;

    strf::encoding_policy epoli
        { strf::error_handling::ignore
        , allow_surrogates };

    std::basic_string<CharT> expected(5, CharT('-'));
    expected.push_back(CharT('x'));
    for(int i = 0; i < 5; ++i)
    {
        expected.push_back(CharT('+'));
    }

    {
        std::size_t count = 10;
        auto result = strf::to_basic_string<CharT>
            .facets(enc, epoli)
            ( strf::multi(CharT('-'), 5)
            , strf::right(CharT('x'), count + 1, fill_char)
            , strf::multi(CharT('+'), 5) );
        BOOST_TEST(result == expected);
    }
}


int main()
{
    {
        // UTF-8

        test_fill(strf::utf8(), 0x7F, "\x7F");
        test_fill(strf::utf8(), 0x80, "\xC2\x80");
        test_fill(strf::utf8(), 0x800, "\xE0\xA0\x80");
        test_fill(strf::utf8(), 0xD800, "\xED\xA0\x80", true);
        test_fill(strf::utf8(), 0xDBFF, "\xED\xAF\xBF", true);
        test_fill(strf::utf8(), 0xDC00, "\xED\xB0\x80", true);
        test_fill(strf::utf8(), 0xDFFF, "\xED\xBF\xBF", true);
        test_fill(strf::utf8(), 0xFFFF, "\xEF\xBF\xBF");
        test_fill(strf::utf8(), 0x10000, "\xF0\x90\x80\x80");
        test_fill(strf::utf8(), 0x10FFFF, "\xF4\x8F\xBF\xBF");


        test_fill(strf::utf8(), 0xD800, "\xEF\xBF\xBD");
        test_fill(strf::utf8(), 0xDBFF, "\xEF\xBF\xBD");
        test_fill(strf::utf8(), 0xDC00, "\xEF\xBF\xBD");
        test_fill(strf::utf8(), 0xDFFF, "\xEF\xBF\xBD");
        test_fill(strf::utf8(), 0x110000, "\xEF\xBF\xBD");
        test_invalid_fill_stop(strf::utf8(), 0x110000);
        test_invalid_fill_ignore(strf::utf8(), 0x110000);
    }

    {
        // UTF-16;
        test_fill(strf::utf16(), U'a', u"a");
        test_fill(strf::utf16(), 0xD800, {0xD800}, true);
        test_fill(strf::utf16(), 0xDBFF, {0xDBFF}, true);
        test_fill(strf::utf16(), 0xDC00, {0xDC00}, true);
        test_fill(strf::utf16(), 0xDFFF, {0xDFFF}, true);
        test_fill(strf::utf16(), 0x10000,  u"\U00010000");
        test_fill(strf::utf16(), 0x10FFFF, u"\U0010FFFF");

        test_fill(strf::utf16(), 0xD800, u"\uFFFD");
        test_fill(strf::utf16(), 0xDBFF, u"\uFFFD");
        test_fill(strf::utf16(), 0xDC00, u"\uFFFD");
        test_fill(strf::utf16(), 0xDFFF, u"\uFFFD");
        test_fill(strf::utf16(), 0x110000, u"\uFFFD");
        test_invalid_fill_stop(strf::utf16(), 0x110000);
        test_invalid_fill_ignore(strf::utf16(), 0x110000);
    }

    {
        // UTF-32;
        test_fill(strf::utf32(), U'a', U"a");
        test_fill(strf::utf32(), 0xD800, {0xD800}, true);
        test_fill(strf::utf32(), 0xDBFF, {0xDBFF}, true);
        test_fill(strf::utf32(), 0xDC00, {0xDC00}, true);
        test_fill(strf::utf32(), 0xDFFF, {0xDFFF}, true);
        test_fill(strf::utf32(), 0x10000,  U"\U00010000");
        test_fill(strf::utf32(), 0x10FFFF, U"\U0010FFFF");

        test_fill(strf::utf32(), 0xD800, U"\uFFFD");
        test_fill(strf::utf32(), 0xDBFF, U"\uFFFD");
        test_fill(strf::utf32(), 0xDC00, U"\uFFFD");
        test_fill(strf::utf32(), 0xDFFF, U"\uFFFD");
        test_fill(strf::utf32(), 0x110000, U"\uFFFD");
        test_invalid_fill_stop(strf::utf32(), 0x110000);
        test_invalid_fill_ignore(strf::utf32(), 0x110000);
    }

    {
        // single byte encodings
        test_fill(strf::windows_1252(), 0x201A, "\x82");
        test_fill(strf::iso_8859_1(), 0x82, "\x82");
        test_fill(strf::iso_8859_3(), 0x02D8, "\xA2");
        test_fill(strf::iso_8859_15(), 0x20AC, "\xA4");

        std::vector<strf::encoding<char>> vec = { strf::windows_1252()
                                                , strf::iso_8859_1()
                                                , strf::iso_8859_3()
                                                , strf::iso_8859_15() };
        for (auto enc : vec)
        {
            test_fill(enc, 'a' , "a");
            test_fill(enc, 0x800, "?");
            test_invalid_fill_stop(enc, 0x800);
            test_invalid_fill_ignore(enc, 0x800);
        }
    }

    return boost::report_errors();
}
