#ifndef STRINGIFY_TEST_TEST_UTILS_HPP_INCLUDED
#define STRINGIFY_TEST_TEST_UTILS_HPP_INCLUDED

//  Distributed under the Boost Software License, Version 1.0.
//  (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)

#include <strf.hpp>
#include <cctype>
#include <algorithm> // for std::generate.

#if defined(_WIN32)
#include <windows.h>
#endif  // defined(_WIN32)

#include "boost/current_function.hpp"

namespace test_utils {

std::string unique_tmp_file_name()
{

#if defined(_WIN32)

    char dirname[MAX_PATH];
    GetTempPathA(MAX_PATH, dirname);
    char fullname[MAX_PATH];
    sprintf_s(fullname, MAX_PATH, "%s\\test_boost_outbuf_%x.txt", dirname, std::rand());
    return fullname;

#else // defined(_WIN32)

   char fullname[200];
   sprintf(fullname, "/tmp/test_boost_outbuf_%x.txt", std::rand());
   return fullname;

#endif  // defined(_WIN32)
}

std::wstring read_wfile(std::FILE* file)
{
    std::wstring result;
    wint_t ch = fgetwc(file);
    while(ch != WEOF) {
        result += static_cast<wchar_t>(ch);
        ch = fgetwc(file);
    };

    return result;
}

std::wstring read_wfile(const char* filename)
{
    std::wstring result;

#if defined(_WIN32)

    std::FILE* file = NULL;
    (void) fopen_s(&file, filename, "r");

#else // defined(_WIN32)

    std::FILE* file = std::fopen(filename, "r");

#endif  // defined(_WIN32)

    if(file != nullptr) {
        result = read_wfile(file);
        fclose(file);
    }
    return result;
}

template <typename CharT>
std::basic_string<CharT> read_file(std::FILE* file)
{
    constexpr std::size_t buff_size = 500;
    CharT buff[buff_size];
    std::basic_string<CharT> result;
    std::size_t read_size = 0;
    do {
        read_size = std::fread(buff, sizeof(buff[0]), buff_size, file);
        result.append(buff, read_size);
    }
    while(read_size == buff_size);

    return result;
}

template <typename CharT>
std::basic_string<CharT> read_file(const char* filename)
{
    std::basic_string<CharT> result;

#if defined(_WIN32)

    std::FILE* file = nullptr;
    (void) fopen_s(&file, filename, "r");

#else // defined(_WIN32)

    std::FILE* file = std::fopen(filename, "r");

#endif  // defined(_WIN32)


    if(file != nullptr) {
        result = read_file<CharT>(file);
    }
    if (file != nullptr) {
        fclose(file);
    }

    return result;
}

template <typename CharT>
struct char_generator
{
    CharT operator()()
    {
        ch = ch == 0x79 ? 0x20 : (ch + 1);
        return ch;
    }
    CharT ch = 0x20;
};

template <typename CharT>
std::basic_string<CharT> make_string(std::size_t size)
{
    std::basic_string<CharT> str(size, CharT('x'));
    char_generator<CharT> gen;
    std::generate(str.begin(), str.end(), gen);
    return str;
}

template <typename CharT>
constexpr std::size_t full_string_size
= strf::min_size_after_recycle<CharT>();

template <typename CharT>
constexpr std::size_t half_string_size = full_string_size<CharT> / 2;

template <typename CharT>
constexpr std::size_t double_string_size = full_string_size<CharT> * 2;

template <typename CharT>
inline strf::detail::simple_string_view<CharT> make_double_string()
{
    enum {arr_size = double_string_size<CharT>};
    static const CharT arr[arr_size]
      = { 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27
        , 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f
        , 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37
        , 0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f
        , 0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47
        , 0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f
        , 0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57
        , 0x58, 0x59, 0x5a, 0x5b, 0x5c, 0x5d, 0x5e, 0x5f
        , 0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67
        , 0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f
        , 0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77
        , 0x78, 0x79, 0x7a, 0x7b, 0x7c, 0x7d, 0x7e, 0x7f

        , 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27
        , 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f
        , 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37
        , 0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f };

    return {arr, arr_size};
}

template <typename CharT>
inline strf::detail::simple_string_view<CharT> make_full_string()
{
    return { make_double_string<CharT>().begin(), full_string_size<CharT> };
}

template <typename CharT>
inline strf::detail::simple_string_view<CharT> make_half_string()
{
    return { make_double_string<CharT>().begin(), half_string_size<CharT> };
}

template <typename CharT>
inline strf::detail::simple_string_view<CharT> make_tiny_string()
{
    static const CharT arr[5] = {'H', 'e', 'l', 'l', 'o'};
    return {arr, 5};
}

template <typename CharT>
inline void turn_into_bad(strf::basic_outbuf<CharT>& ob)
{
    strf::detail::outbuf_test_tool::turn_into_bad(ob.as_underlying());
}

int& test_err_count()
{
    static int count = 0;
    return count;
}

strf::narrow_cfile_writer<char>& test_outbuf()
{
    static strf::narrow_cfile_writer<char> ob(stdout);
    return ob;
}

class test_scope
{
public:

    test_scope(const test_scope&) = delete;

    test_scope()
        : parent_(curr_test_scope())
    {
        parent_->child_ = this;
        curr_test_scope() = this;
        description_[0] = '\0';
    }

    ~test_scope()
    {
        if (parent_) {
            parent_->child_ = child_;
            curr_test_scope() = parent_;
        }
        if (child_) {
            child_ -> parent_ = parent_;
        }
    }

    auto description_writer()
    {
        return strf::to(description_);
    }

    static void print_stack(strf::outbuf& out)
    {
        test_scope* first = root().child_;
        if (first != nullptr) {
            strf::write(out, "\n    ( ");
            strf::write(out, first->description_);
            for(auto it = first->child_; it != nullptr; it = it->child_) {
                strf::write(out, " / ");
                strf::write(out, it->description_);
            }
            strf::write(out, " )");
        }
    }

private:

    struct root_tag {};

    test_scope(root_tag)
    {
        description_[0] = '\0';
    }

    static test_scope& root()
    {
        static test_scope r{test_scope::root_tag{}};
        return r;
    }

    static test_scope*& curr_test_scope()
    {
        static test_scope* curr = &root();
        return curr;
    }


    test_scope* parent_ = nullptr;
    test_scope* child_ = nullptr;
    char description_[200];
};

template <typename ... Args>
auto test_message
    ( const char* filename
    , int line
    , const char* funcname
    , const Args& ... args )
{
    to(test_outbuf()) (filename, ':', line, ": ", args...);
    test_scope::print_stack(test_outbuf());
    to(test_outbuf()) ("\n    In function '", funcname, "'\n");
}


template <typename ... Args>
inline auto test_failure
    ( const char* filename
    , int line
    , const char* funcname
    , const Args& ... args )
{
    ++ test_err_count();
    test_message(filename, line, funcname, args...);
}

template <typename CharOut>
class input_tester
    : public strf::basic_outbuf<CharOut>
{

public:

    input_tester
        ( strf::detail::simple_string_view<CharOut> expected
        , const char* src_filename
        , int src_line
        , const char* function
        , double reserve_factor
        , std::size_t size = 0 );

#ifdef STRF_NO_CXX17_COPY_ELISION

    input_tester(input_tester&& r);

#else

    input_tester(input_tester&& r) = delete;
    input_tester(const input_tester& r) = delete;

#endif

    ~input_tester();

    using char_type = CharOut;

    void recycle() override;

    void finish();

private:

    template <typename ... MsgArgs>
    void _test_failure(const MsgArgs&... msg_args)
    {
        test_utils::test_failure( _src_filename, _src_line
                                , _function, msg_args... );
    }

    bool _wrongly_reserved(std::size_t result_size) const;

    strf::detail::simple_string_view<CharOut> _expected;
    std::size_t _reserved_size;
    const char* _src_filename;
    const char* _function;
    int _src_line;
    double _reserve_factor;

    bool _expect_error = false;
    bool _recycle_called = false;
    bool _source_location_printed = false;

    CharOut* _pos_before_overflow = nullptr;
    //constexpr static std::size_t _buffer_size = 500;
    enum {_buffer_size = 500};
    CharOut _buffer[_buffer_size];
};


template <typename CharOut>
input_tester<CharOut>::input_tester
    ( strf::detail::simple_string_view<CharOut> expected
    , const char* src_filename
    , int src_line
    , const char* function
    , double reserve_factor
    , std::size_t size )
    : strf::basic_outbuf<CharOut>{_buffer, size}
    , _expected(expected)
    , _reserved_size(size)
    , _src_filename(std::move(src_filename))
    , _function(function)
    , _src_line(src_line)
    , _reserve_factor(reserve_factor)
{
    if (size > _buffer_size) {
        test_utils::test_message
            ( _src_filename, _src_line, _function
            , "Warning: reserved more characters (", size
            , ") then the tester buffer size (", _buffer_size, ")." );
        this->set_end(_buffer + _buffer_size);
    }
}

template <typename CharOut>
input_tester<CharOut>::~input_tester()
{
}

template <typename CharOut>
void input_tester<CharOut>::recycle()
{
    _test_failure(" basic_outbuf::recycle() called "
                  "( calculated size too small ).\n");

    if ( this->pos() + strf::min_size_after_recycle<CharOut>()
       > _buffer + _buffer_size )
    {
        _pos_before_overflow = this->pos();
        this->set_pos(strf::outbuf_garbage_buf<CharOut>());
        this->set_end(strf::outbuf_garbage_buf_end<CharOut>());
    } else {
        this->set_end(_buffer + _buffer_size);
    }
}

template <typename CharOut>
void input_tester<CharOut>::finish()
{
    auto pos = _pos_before_overflow ? _pos_before_overflow : this->pos();
    strf::detail::simple_string_view<CharOut> result{_buffer, pos};

    if ( result.size() != _expected.size()
      || 0 != std::char_traits<CharOut>::compare( _expected.begin()
                                                , result.begin()
                                                , _expected.size() ))
    {
        _test_failure( "\n  expected: \"", strf::cv(_expected), '\"'
                     , "\n  obtained: \"", strf::cv(result), "\"" );

    }
    if(_wrongly_reserved(result.size())) {
        _test_failure( "\n  reserved size  : ", _reserved_size
                     , "\n  necessary size : ", result.size() );
    }
}

template <typename CharOut>
bool input_tester<CharOut>::_wrongly_reserved(std::size_t result_size) const
{
    return ( _reserved_size < result_size
          || ( ( static_cast<double>(_reserved_size)
               / static_cast<double>(result_size) )
             > _reserve_factor ) );
}

template <typename CharT>
class input_tester_creator
{
public:

    using char_type = CharT;

    input_tester_creator( strf::detail::simple_string_view<CharT> expected
                        , const char* filename
                        , int line
                        , const char* function
                        , double reserve_factor )
        : _expected(expected)
        , _filename(filename)
        , _function(function)
        , _line(line)
        , _reserve_factor(reserve_factor)
    {
    }

    input_tester_creator(const input_tester_creator& ) = default;
    input_tester_creator(input_tester_creator&& ) = default;

    test_utils::input_tester<CharT> create(std::size_t size) const
    {
        return test_utils::input_tester<CharT>
            { _expected, _filename, _line, _function, _reserve_factor, size };
    }

private:

    strf::detail::simple_string_view<CharT> _expected;
    const char* _filename;
    const char* _function;
    int _line;
    double _reserve_factor = 1.0;
};

template<typename CharT>
auto make_tester
   ( const CharT* expected
   , const char* filename
   , int line
   , const char* function
   , double reserve_factor = 1.0 )
{
    return strf::destination_calc_size
        < test_utils::input_tester_creator<CharT> >
        ( expected, filename, line, function, reserve_factor);
}

template<typename CharT>
auto make_tester
   ( strf::detail::simple_string_view<CharT> expected
   , const char* filename
   , int line
   , const char* function
   , double reserve_factor = 1.0 )
{
    return strf::destination_calc_size
        < test_utils::input_tester_creator<CharT> >
        ( expected, filename, line, function, reserve_factor);
}

#if defined(_MSC_VER)
# pragma warning(push)
# pragma warning(disable: 4389)
#elif defined(__clang__) && defined(__has_warning)
# if __has_warning("-Wsign-compare")
#  pragma clang diagnostic push
#  pragma clang diagnostic ignored "-Wsign-compare"
# endif
#elif defined(__GNUC__) && !(defined(__INTEL_COMPILER) || defined(__ICL) || defined(__ICC) || defined(__ECC)) && (__GNUC__ * 100 + __GNUC_MINOR__) >= 406
# pragma GCC diagnostic push
# pragma GCC diagnostic ignored "-Wsign-compare"
#endif


template <typename T, typename U>
constexpr bool equal(const T&a, const U&b)
{
    return a == b;
}


#if defined(_MSC_VER)
# pragma warning(pop)
#elif defined(__clang__) && defined(__has_warning)
# if __has_warning("-Wsign-compare")
#  pragma clang diagnostic pop
# endif
#elif defined(__GNUC__) && !(defined(__INTEL_COMPILER) || defined(__ICL) || defined(__ICC) || defined(__ECC)) && (__GNUC__ * 100 + __GNUC_MINOR__) >= 406
# pragma GCC diagnostic pop
#endif


} // namespace test_utils

#define TEST(EXPECTED)                                                  \
    test_utils::make_tester( (EXPECTED), __FILE__, __LINE__             \
                           , BOOST_CURRENT_FUNCTION)

#define TEST_RF(EXPECTED, RF)                                           \
    test_utils::make_tester( (EXPECTED), __FILE__, __LINE__             \
                           , BOOST_CURRENT_FUNCTION, (RF))

#define TEST_STR_CONCAT_2(str1, str2) str1 ## str2

#define TEST_STR_CONCAT(str1, str2) TEST_STR_CONCAT_2(str1, str2)

#define TEST_SCOPE_DESCRIPTION                                          \
    test_utils::test_scope TEST_STR_CONCAT(test_label_, __LINE__);      \
    TEST_STR_CONCAT(test_label_, __LINE__).description_writer()

#define TEST_ERROR(msg) \
    test_utils::test_failure(__FILE__, __LINE__, BOOST_CURRENT_FUNCTION, (msg));

#define TEST_TRUE(expr)                                                 \
    if (!(expr))                                                        \
        test_utils::test_failure                                        \
            ( __FILE__, __LINE__, BOOST_CURRENT_FUNCTION                \
            , "test (" #expr ") failed. " );                            \

#define TEST_EQ(a, b)                                                   \
    if (!test_utils::equal((a), (b)))                                   \
        test_utils::test_failure                                        \
            ( __FILE__, __LINE__, BOOST_CURRENT_FUNCTION                \
            , " test (", (a), " == ", (b), ") failed. " );

#define TEST_CSTR_EQ(a, b)                                              \
    if (0 != std::strcmp(a, b))                                         \
        test_utils::test_failure                                        \
            ( __FILE__, __LINE__, BOOST_CURRENT_FUNCTION                \
            , "test (s1 == s2) failed. Where:\n    s1 is \"", (a)       \
            , "\"\n    s2 is \"", (b), '\"' );

#define TEST_THROWS( EXPR, EXCEP )                                      \
  { bool caught = false;                                                \
    try { EXPR; }                                                       \
    catch(EXCEP const&) { caught = true; }                              \
    if (!caught)                                                        \
          test_utils::test_failure                                      \
              ( __FILE__, __LINE__, BOOST_CURRENT_FUNCTION              \
              , "exception " #EXCEP " not thrown as expected" );        \
  }

int test_finish()
{
    int err_count = test_utils::test_err_count();
    if (err_count == 0) {
        strf::write(test_utils::test_outbuf(), "All test passed!\n");
    } else {
        strf::to(test_utils::test_outbuf()) (err_count, " tests failed!\n");
    }
    test_utils::test_outbuf().finish();
    return err_count;
}

#endif
