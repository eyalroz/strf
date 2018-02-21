//  Distributed under the Boost Software License, Version 1.0.
//  (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)

#define  _CRT_SECURE_NO_WARNINGS

#include <boost/detail/lightweight_test.hpp>
#include "test_utils.hpp"
#include "error_code_emitter_arg.hpp"
#include "exception_thrower_arg.hpp"
#include <boost/stringify.hpp>

namespace strf = boost::stringify::v0;

template <typename CharT>
void basic_test()
{
    CharT result[100];
    std::fill(result, result + 100, CharT('-'));
    std::size_t result_length = 0;
    std::basic_string<CharT> expected;

    std::error_code err = use_all_writing_function_of_output_writer
        ( strf::write_to(result, &result_length)
          , expected );

    BOOST_TEST(!err);
    BOOST_TEST(expected.length() == result_length);
    BOOST_TEST(expected == result);
}


template <typename CharT>
void test_array_too_small()
{
    CharT buff[3] = { 'a', 'a', 0 };
    std::size_t result_length = 1000;

    std::error_code ec = strf::write_to(buff, &result_length) = { 1234567 };

    BOOST_TEST(buff[0] == 0);
    BOOST_TEST(result_length == 0);
    BOOST_TEST(ec == std::errc::result_out_of_range);
}

template <typename CharT>
void test_informed_size_too_small()
{
    CharT buff[100] = { 'a', 'a', 0 };
    std::size_t result_length = 1000;

    std::error_code ec = strf::write_to(buff, 3, &result_length) = { 1234567 };

    BOOST_TEST(buff[0] == 0);
    BOOST_TEST(result_length == 0);
    BOOST_TEST(ec == std::errc::result_out_of_range);
}

template <typename CharT>
void test_informed_end_too_close()
{
    CharT buff[100] = { 'a', 'a', 0 };
    std::size_t result_length = 1000;

    std::error_code ec = strf::write_to(buff, &buff[3], &result_length) = { 1234567 };

    BOOST_TEST(buff[0] == 0);
    BOOST_TEST(result_length == 0);
    BOOST_TEST(ec == std::errc::result_out_of_range);
}

int main()
{
    basic_test<char>();
    basic_test<char16_t>();
    basic_test<char32_t>();
    basic_test<wchar_t>();

    {  // Test alternative char traits

        char16_t result[200] = u"";
        std::size_t result_length = 10000;
        using traits = to_upper_char_traits<char16_t>;

        auto err = strf::write_to<traits>(result, &result_length)
            = {u'a', strf::multi(u'b', 3), u"zzz", ~strf::hex(10)};

        const std::u16string expected = u"ABBBZZZ0XA";
        BOOST_TEST(!err);
        BOOST_TEST(expected.length() == result_length);
        BOOST_TEST(expected == result);
    }

    {
        // TODO: Test alternative char traits in repeat(ch, ch, ...)
    }

    {   // Test char_ptr_writer::set_error
        //
        // When set_error(some_err) is called, some_err is returned at the end
        
        char16_t result[200] = u"-----------------------------";
        std::size_t result_length = 1000;

        std::error_code ec = strf::write_to(result, &result_length)
            = {u"abcd", error_code_emitter_arg, u"lkjlj"};

        BOOST_TEST(result[0] == u'\0');
        BOOST_TEST(result_length == 0);
        BOOST_TEST(ec == std::errc::invalid_argument);
    }

    {  // When exception is thrown 

        char16_t result[200] = u"-----------------------------";
        std::size_t result_length = 1000;

        try
        {
            strf::write_to(result, &result_length)
                &= {u"abcd", exception_thrower_arg, u"lkjlj"};
        }
        catch(...)
        {
        }

        BOOST_TEST(result[0] == u'\0');
        BOOST_TEST(result_length == 0);
    }
    
    test_array_too_small<char>();
    test_array_too_small<char16_t>();
    test_array_too_small<char32_t>();
    test_array_too_small<wchar_t>();

    test_informed_size_too_small<char>();
    test_informed_size_too_small<char16_t>();
    test_informed_size_too_small<char32_t>();
    test_informed_size_too_small<wchar_t>();

    test_informed_end_too_close<char>();
    test_informed_end_too_close<char16_t>();
    test_informed_end_too_close<char32_t>();
    test_informed_end_too_close<wchar_t>();
   
    {   // When overflow happens in char_ptr_writer::put(str, count)

        char16_t result[200] = u"--------------------------------------------------";
        std::size_t result_length = 1000;

        std::error_code ec = strf::write_to(result, 3, &result_length) = { u"abc" };

        BOOST_TEST(result[0] == u'\0');
        BOOST_TEST(result_length == 0);
        BOOST_TEST(ec == std::errc::result_out_of_range);
    }

    {   // When overflow happens in char_ptr_writer::put(ch)

        char16_t result[200] = u"--------------------------------------------------";
        std::size_t result_length = 1000;

        std::error_code ec = strf::write_to(result, 3, &result_length) = { u'a', u'b', u'c' };

        BOOST_TEST(result[0] == u'\0');
        BOOST_TEST(result_length == 0);
        BOOST_TEST(ec == std::errc::result_out_of_range);
    }

    
   {   // When overflow happens in char_ptr_writer::repeat(ch, count)

       char result[200] = "--------------------------------------------------";
       std::size_t result_length = 1000;
       std::error_code ec = strf::write_to(result, 2, &result_length)
           = {strf::multi('x', 10)};
       BOOST_TEST(result[0] == '\0');
       BOOST_TEST(result_length == 0);
       BOOST_TEST(ec == std::errc::result_out_of_range);
   }
   {   // When overflow happens in char_ptr_writer::repeat(ch, ch, count)

       char result[3] = "";
       std::size_t result_length = 1000;
       std::error_code ec = strf::write_to(result, 3, &result_length)
           = {strf::multi(U'\u0080', 2)};

       BOOST_TEST(result[0] == '\0');
       BOOST_TEST(result_length == 0);
       BOOST_TEST(ec == std::errc::result_out_of_range);
   }
   {   // When overflow happens in char_ptr_writer::repeat(ch, ch, ch, count)

       char result[200] = "--------------------------------------------------";
       std::size_t result_length = 1000;
       std::error_code ec = strf::write_to(result, 5, &result_length)
           = {strf::multi(U'\u0800', 2)};

       BOOST_TEST(result[0] == '\0');
       BOOST_TEST(result_length == 0);
       BOOST_TEST(ec == std::errc::result_out_of_range);
   }
   {   // When overflow happens in char_ptr_writer::repeat(ch, ch, ch, ch, count)

       char result[200] = "--------------------------------------------------";
       std::size_t result_length = 1000;
       std::error_code ec = strf::write_to(result, 7, &result_length)
           = {strf::multi(U'\U00010000', 2)};

       BOOST_TEST(result[0] == '\0');
       BOOST_TEST(result_length == 0);
       BOOST_TEST(ec == std::errc::result_out_of_range);
   }

    int rc = report_errors() || boost::report_errors();
    return rc;
}
