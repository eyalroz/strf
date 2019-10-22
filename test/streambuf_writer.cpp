//  Distributed under the Boost Software License, Version 1.0.
//  (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)

#include "test_utils.hpp"
#include <sstream>

namespace strf = boost::stringify::v0;

template <typename CharT>
void test_successfull_writing()
{
    auto tiny_str = test_utils::make_tiny_string<CharT>();
    auto double_str = test_utils::make_double_string<CharT>();

    std::basic_ostringstream<CharT> dest;
    strf::basic_streambuf_writer<CharT> writer(*dest.rdbuf());

    auto expected_content = tiny_str + double_str;

    write(writer, tiny_str.data(), tiny_str.size());
    write(writer, double_str.data(), double_str.size());
    auto status = writer.finish();
    dest.rdbuf()->pubsync();

    auto obtained_content = dest.str();

    BOOST_TEST(status.success);
    BOOST_TEST_EQ(status.count, obtained_content.size());
    BOOST_TEST(obtained_content == expected_content);
}

template <typename CharT>
void test_failing_to_recycle()
{
    auto half_str = test_utils::make_half_string<CharT>();
    auto double_str = test_utils::make_double_string<CharT>();
    auto expected_content = half_str;

    std::basic_ostringstream<CharT> dest;
    strf::basic_streambuf_writer<CharT> writer(*dest.rdbuf());

    write(writer, half_str.data(), half_str.size());
    writer.recycle(); // first recycle works
    test_utils::turn_into_bad(writer);
    write(writer, double_str.data(), double_str.size());
    auto status = writer.finish();
    dest.rdbuf()->pubsync();

    auto obtained_content = dest.str();

    BOOST_TEST(! status.success);
    BOOST_TEST_EQ(status.count, obtained_content.size());
    BOOST_TEST(obtained_content == expected_content);
}

template <typename CharT>
void test_failing_to_finish()
{
    auto double_str = test_utils::make_double_string<CharT>();
    auto half_str = test_utils::make_half_string<CharT>();
    auto expected_content = double_str;

    std::basic_ostringstream<CharT> dest;
    strf::basic_streambuf_writer<CharT> writer(*dest.rdbuf());

    write(writer, double_str.data(), double_str.size());
    writer.recycle();
    write(writer, half_str.data(), half_str.size());
    test_utils::turn_into_bad(writer);

    auto status = writer.finish();
    dest.rdbuf()->pubsync();

    auto obtained_content = dest.str();

    BOOST_TEST(! status.success);
    BOOST_TEST_EQ(status.count, obtained_content.size());
    BOOST_TEST(obtained_content == expected_content);
}

template <typename CharT>
void test_dispatcher()
{
    auto half_str = test_utils::make_half_string<CharT>();
    auto full_str = test_utils::make_full_string<CharT>();

    {
        std::basic_ostringstream<CharT> dest;
        strf::write(dest.rdbuf()) (half_str, full_str);
        BOOST_TEST(dest.str() == half_str + full_str);
    }
    {
        std::basic_ostringstream<CharT> dest;
        strf::write(*dest.rdbuf()) (half_str, full_str);
        BOOST_TEST(dest.str() == half_str + full_str);
    }
}

int main()
{
    test_dispatcher<char>();
    test_dispatcher<char16_t>();
    test_dispatcher<char32_t>();
    test_dispatcher<wchar_t>();

    test_successfull_writing<char>();
    test_successfull_writing<char16_t>();
    test_successfull_writing<char32_t>();
    test_successfull_writing<wchar_t>();

    test_failing_to_recycle<char>();
    test_failing_to_recycle<char16_t>();
    test_failing_to_recycle<char32_t>();
    test_failing_to_recycle<wchar_t>();

    test_failing_to_finish<char>();
    test_failing_to_finish<char16_t>();
    test_failing_to_finish<char32_t>();
    test_failing_to_finish<wchar_t>();

    return boost::report_errors();
}
