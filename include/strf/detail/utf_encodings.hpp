#ifndef STRF_DETAIL_UTF_ENCODINGS_HPP
#define STRF_DETAIL_UTF_ENCODINGS_HPP

//  Distributed under the Boost Software License, Version 1.0.
//  (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)

#include <strf/printer.hpp>
#include <algorithm>
#include <cstring>

STRF_NAMESPACE_BEGIN

#define STRF_CHECK_DEST     \
    if (dest_it == dest_end) {         \
        ob.advance_to(dest_it);        \
        ob.recycle();                  \
        if (!ob.good()) {              \
            return;                    \
        }                              \
        dest_it = ob.pos();            \
        dest_end = ob.end();           \
    }

#define STRF_CHECK_DEST_SIZE(SIZE)     \
    if (dest_it + SIZE > dest_end) {              \
        ob.advance_to(dest_it);                   \
        ob.recycle();                             \
        if (!ob.good()) {                         \
            return;                               \
        }                                         \
        dest_it = ob.pos();                       \
        dest_end = ob.end();                      \
    }

namespace detail {

template <typename T, std::size_t N>
struct simple_array;

template <typename T> struct simple_array<T,1> { T obj0; };
template <typename T> struct simple_array<T,2> { T obj0;  T obj1; };
template <typename T> struct simple_array<T,3> { T obj0;  T obj1; T obj2; };
template <typename T> struct simple_array<T,4> { T obj0;  T obj1; T obj2; T obj3; };

#ifdef __CUDA_ARCH__

template<class OutputIt, class Size, class T>
OutputIt STRF_HD fill_n(OutputIt first, Size count, const T& value)
{
	auto it = first;
    for (Size i = 0; i != count; ++i, ++first) {
        *it = value;
    }
    return it;
}

#endif

template <typename CharT, std::size_t N>
inline STRF_HD void do_repeat_sequence
    ( CharT* dest
    , std::size_t count
    , simple_array<CharT, N> seq )
{
#ifndef __CUDA_ARCH__
	using std::fill_n;
#endif
    fill_n(reinterpret_cast<simple_array<CharT, N>*>(dest), count, seq);
}

template <typename CharT, std::size_t N>
STRF_HD void repeat_sequence_continuation
    ( strf::underlying_outbuf<sizeof(CharT)>& ob
    , std::size_t count
    , simple_array<CharT, N> seq )
{
    std::size_t space = ob.size() / N;
    STRF_ASSERT(space < count);

    strf::detail::do_repeat_sequence(ob.pos(), space, seq);
    count -= space;
    ob.advance(space * N);
    ob.recycle();
    while (true)//ob.good())
    {
        space = ob.size() / N;
        if (count <= space)
        {
            strf::detail::do_repeat_sequence(ob.pos(), count, seq);
            ob.advance(count * N);
            return;
        }
        strf::detail::do_repeat_sequence(ob.pos(), space, seq);
        count -= space;
        ob.advance(space * N);
        ob.recycle();
    }
}

template <typename CharT, std::size_t N>
inline STRF_HD void repeat_sequence
    ( strf::underlying_outbuf<sizeof(CharT)>& ob
    , std::size_t count
    , simple_array<CharT, N> seq )
{
    if (count * N <= ob.size())
    {
        strf::detail::do_repeat_sequence(ob.pos(), count, seq);
        ob.advance(count * N);
    }
    else
    {
        strf::detail::repeat_sequence_continuation(ob, count, seq);
    }
}

constexpr STRF_HD bool is_surrogate(std::uint32_t codepoint)
{
    return codepoint >> 11 == 0x1B;
}
constexpr STRF_HD bool is_high_surrogate(std::uint32_t codepoint) noexcept
{
    return codepoint >> 10 == 0x36;
}
constexpr STRF_HD bool is_low_surrogate(std::uint32_t codepoint) noexcept
{
    return codepoint >> 10 == 0x37;
}
constexpr STRF_HD bool not_surrogate(std::uint32_t codepoint)
{
    return codepoint >> 11 != 0x1B;
}
constexpr STRF_HD  bool not_high_surrogate(std::uint32_t codepoint)
{
    return codepoint >> 10 != 0x36;
}
constexpr STRF_HD  bool not_low_surrogate(std::uint32_t codepoint)
{
    return codepoint >> 10 != 0x37;
}
constexpr STRF_HD std::uint16_t utf8_decode(std::uint16_t ch0, std::uint16_t ch1)
{
    return (((ch0 & 0x1F) << 6) |
            ((ch1 & 0x3F) << 0));
}
constexpr STRF_HD std::uint16_t utf8_decode(std::uint16_t ch0, std::uint16_t ch1, std::uint16_t ch2)
{
    return (((ch0 & 0x0F) << 12) |
            ((ch1 & 0x3F) <<  6) |
            ((ch2 & 0x3F) <<  0));
}
constexpr STRF_HD std::uint32_t utf8_decode(std::uint32_t ch0, std::uint32_t ch1, std::uint32_t ch2, std::uint32_t ch3)
{
    return (((ch0 & 0x07) << 18) |
            ((ch1 & 0x3F) << 12) |
            ((ch2 & 0x3F) <<  6) |
            ((ch3 & 0x3F) <<  0));
}
constexpr STRF_HD bool is_utf8_continuation(std::uint8_t ch)
{
    return (ch & 0xC0) == 0x80;
}

constexpr STRF_HD bool valid_start_3bytes
    ( std::uint8_t ch0
    , std::uint8_t ch1
    , strf::surrogate_policy allow_surr )
{
    return ( (ch0 != 0xE0 || ch1 != 0x80)
          && ( allow_surr == strf::surrogate_policy::lax
            || (0x1B != (((ch0 & 0xF) << 1) | ((ch1 >> 5) & 1)))) );
}

inline STRF_HD unsigned utf8_decode_first_2_of_3(std::uint8_t ch0, std::uint8_t ch1)
{
    return ((ch0 & 0x0F) << 6) | (ch1 & 0x3F);
}

inline STRF_HD bool first_2_of_3_are_valid( unsigned x
                                  , strf::surrogate_policy allow_surr )
{
    return ( allow_surr == strf::surrogate_policy::lax
          || (x >> 5) != 0x1B );
}
inline STRF_HD bool first_2_of_3_are_valid( std::uint8_t ch0
                                  , std::uint8_t ch1
                                  , strf::surrogate_policy allow_surr )
{
    return first_2_of_3_are_valid(utf8_decode_first_2_of_3(ch0, ch1), allow_surr);
}

inline STRF_HD unsigned utf8_decode_first_2_of_4(std::uint8_t ch0, std::uint8_t ch1)
{
    return ((ch0 & 0x07) << 6) | (ch1 & 0x3F);
}

inline STRF_HD unsigned utf8_decode_last_2_of_4(unsigned long x, unsigned ch2, unsigned ch3)
{
    return (x << 12) | ((ch2 & 0x3F) <<  6) | (ch3 & 0x3F);
}

inline STRF_HD bool first_2_of_4_are_valid(unsigned x)
{
    return 0xF < x && x < 0x110;
}

inline STRF_HD bool first_2_of_4_are_valid(std::uint8_t ch0, std::uint8_t ch1)
{
    return first_2_of_4_are_valid(utf8_decode_first_2_of_4(ch0, ch1));
}

STRF_STATIC_LINKAGE
STRF_HD void utf8_to_utf32_transcode
    ( strf::underlying_outbuf<4>& ob
    , const std::uint8_t* src
    , const std::uint8_t* src_end
    , strf::encoding_error err_hdl
    , strf::surrogate_policy allow_surr )
{
    using strf::detail::utf8_decode;
    using strf::detail::is_utf8_continuation;

    std::uint8_t ch0, ch1, ch2, ch3;
    unsigned long x;
    auto src_it = src;
    auto dest_it = ob.pos();
    auto dest_end = ob.end();
    char32_t ch32;

    while(src_it != src_end)
    {
        ch0 = (*src_it);
        ++src_it;
        if (ch0 < 0x80)
        {
            //STRF_CHECK_DEST;
            ch32 = ch0;
        }
        else if (0xC0 == (ch0 & 0xE0))
        {
            if(ch0 > 0xC1 && src_it != src_end && is_utf8_continuation(ch1 = * src_it))
            {
                //STRF_CHECK_DEST;
                ch32 = utf8_decode(ch0, ch1);
                ++src_it;
            } else goto invalid_sequence;
        }
        else if (0xE0 == ch0)
        {
            if (   src_it != src_end && (((ch1 = * src_it) & 0xE0) == 0xA0)
              && ++src_it != src_end && is_utf8_continuation(ch2 = * src_it) )
            {
                //STRF_CHECK_DEST;
                ch32 = ((ch1 & 0x3F) << 6) | (ch2 & 0x3F);
                ++src_it;
            } else goto invalid_sequence;
        }
        else if (0xE0 == (ch0 & 0xF0))
        {
            if (   src_it != src_end && is_utf8_continuation(ch1 = * src_it)
              && first_2_of_3_are_valid( x = utf8_decode_first_2_of_3(ch0, ch1)
                                       , allow_surr )
              && ++src_it != src_end && is_utf8_continuation(ch2 = * src_it) )
            {
                //STRF_CHECK_DEST;
                ch32 = (x << 6) | (ch2 & 0x3F);
                ++src_it;
            } else goto invalid_sequence;
        }
        else if (0xEF < ch0)
        {
            if ( src_it != src_end && is_utf8_continuation(ch1 = * src_it)
              && first_2_of_4_are_valid(x = utf8_decode_first_2_of_4(ch0, ch1))
              && ++src_it != src_end && is_utf8_continuation(ch2 = * src_it)
              && ++src_it != src_end && is_utf8_continuation(ch3 = * src_it) )
            {
                //STRF_CHECK_DEST;
                ch32 = utf8_decode_last_2_of_4(x, ch2, ch3);
                ++src_it;
            } else goto invalid_sequence;
        }
        else
        {
            invalid_sequence:
            if (err_hdl == strf::encoding_error::stop)
            {
                ob.advance_to(dest_it);
                strf::detail::throw_encoding_failure();
            }
            ch32 = 0xFFFD;
        }

        STRF_CHECK_DEST;
        *dest_it = ch32;
        ++dest_it;
    }
    ob.advance_to(dest_it);
}

STRF_STATIC_LINKAGE STRF_HD std::size_t utf8_to_utf32_size
    ( const std::uint8_t* src
    , const std::uint8_t* src_end
    , strf::surrogate_policy allow_surr )
{
    std::uint8_t ch0, ch1;
    const std::uint8_t* src_it = src;
    std::size_t size = 0;
    while (src_it != src_end)
    {
        ch0 = (*src_it);
        ++src_it;
        ++size;
        if (0xC0 == (ch0 & 0xE0))
        {
            if (ch0 > 0xC1 && src_it != src_end && is_utf8_continuation(*src_it))
            {
                ++src_it;
            }
        }
        else if (0xE0 == ch0)
        {
            if (   src_it != src_end && (((ch1 = * src_it) & 0xE0) == 0xA0)
              && ++src_it != src_end && is_utf8_continuation(* src_it) )
            {
                ++src_it;
            }
        }
        else if (0xE0 == (ch0 & 0xF0))
        {
            if ( src_it != src_end && is_utf8_continuation(ch1 = * src_it)
              && first_2_of_3_are_valid( ch0, ch1, allow_surr )
              && ++src_it != src_end && is_utf8_continuation(* src_it) )
            {
                ++src_it;
            }
        }
        else if(0xEF < ch0)
        {
            if (   src_it != src_end && is_utf8_continuation(ch1 = * src_it)
              && first_2_of_4_are_valid(ch0, ch1)
              && ++src_it != src_end && is_utf8_continuation(*src_it)
              && ++src_it != src_end && is_utf8_continuation(*src_it) )
            {
                ++src_it;
            }
        }
    }
    return size;
}

STRF_STATIC_LINKAGE STRF_HD void utf8_sanitize
    ( strf::underlying_outbuf<1>& ob
    , const std::uint8_t* src
    , const std::uint8_t* src_end
    , strf::encoding_error err_hdl
    , strf::surrogate_policy allow_surr )
{
    using strf::detail::utf8_decode;

    std::uint8_t ch0, ch1, ch2, ch3;
    auto src_it = src;
    auto dest_it = ob.pos();
    auto dest_end = ob.end();
    while(src_it != src_end)
    {
        ch0 = (*src_it);
        ++src_it;
        if(ch0 < 0x80)
        {
            STRF_CHECK_DEST;
            *dest_it = ch0;
            ++dest_it;
        }
        else if(0xC0 == (ch0 & 0xE0))
        {
            if(ch0 > 0xC1 && src_it != src_end && is_utf8_continuation(ch1 = * src_it))
            {
                STRF_CHECK_DEST_SIZE(2);
                ++src_it;
                dest_it[0] = ch0;
                dest_it[1] = ch1;
                dest_it += 2;
            } else goto invalid_sequence;
        }
        else if (0xE0 == ch0)
        {
            if (   src_it != src_end && (((ch1 = * src_it) & 0xE0) == 0xA0)
              && ++src_it != src_end && is_utf8_continuation(ch2 = * src_it) )
            {
                STRF_CHECK_DEST_SIZE(3);
                ++src_it;
                dest_it[0] = ch0;
                dest_it[1] = ch1;
                dest_it[2] = ch2;
                dest_it += 3;
            } else goto invalid_sequence;
        }
        else if (0xE0 == (ch0 & 0xF0))
        {
            if (   src_it != src_end && is_utf8_continuation(ch1 = * src_it)
              && first_2_of_3_are_valid(ch0, ch1, allow_surr)
              && ++src_it != src_end && is_utf8_continuation(ch2 = * src_it) )
            {
                STRF_CHECK_DEST_SIZE(3);
                ++src_it;
                dest_it[0] = ch0;
                dest_it[1] = ch1;
                dest_it[2] = ch2;
                dest_it += 3;
            } else goto invalid_sequence;
        }
        else if (0xF0 == (ch0 & 0xF8))
        {
            if ( src_it != src_end && is_utf8_continuation(ch1 = * src_it)
              && first_2_of_4_are_valid(ch0, ch1)
              && ++src_it != src_end && is_utf8_continuation(ch2 = * src_it)
              && ++src_it != src_end && is_utf8_continuation(ch3 = * src_it) )
            {
                STRF_CHECK_DEST_SIZE(4);
                ++src_it;
                dest_it[0] = ch0;
                dest_it[1] = ch1;
                dest_it[2] = ch2;
                dest_it[3] = ch3;
                dest_it += 4;
            } else goto invalid_sequence;
        }
        else
        {
            invalid_sequence:
            if (err_hdl == strf::encoding_error::replace)
            {
                STRF_CHECK_DEST_SIZE(3);
                dest_it[0] = 0xEF;
                dest_it[1] = 0xBF;
                dest_it[2] = 0xBD;
                dest_it += 3;
            }
            else
            {
                STRF_ASSERT(err_hdl == strf::encoding_error::stop);
                ob.advance_to(dest_it);
                strf::detail::throw_encoding_failure();
            }
        }
    }
    ob.advance_to(dest_it);
}

STRF_STATIC_LINKAGE STRF_HD std::size_t utf8_sanitize_size
    ( const std::uint8_t* src
    , const std::uint8_t* src_end
    , strf::surrogate_policy allow_surr )
{
    using strf::detail::utf8_decode;
    std::uint8_t ch0, ch1;
    const std::uint8_t* src_it = src;
    std::size_t size = 0;
    while(src_it != src_end)
    {
        ch0 = *src_it;
        ++src_it;
        if(ch0 < 0x80)
        {
            ++size;
        }
        else if (0xC0 == (ch0 & 0xE0))
        {
            if (ch0 > 0xC1 && src_it != src_end && is_utf8_continuation(*src_it))
            {
                size += 2;
                ++src_it;
            }
            else
            {
                size += 3;
            }
        }
        else if (0xE0 == ch0)
        {
            if (   src_it != src_end && (((ch1 = * src_it) & 0xE0) == 0xA0)
              && ++src_it != src_end && is_utf8_continuation(* src_it) )
            {
                size += 3;
                ++src_it;
            }
            else
            {
                size += 3;
            }
        }
        else if (0xE0 == (ch0 & 0xF0))
        {
            size += 3;
            if ( src_it != src_end && is_utf8_continuation(ch1 = * src_it)
              && first_2_of_3_are_valid( ch0, ch1, allow_surr )
              && ++src_it != src_end && is_utf8_continuation(* src_it) )
            {
                ++src_it;
            }
        }
        else if( 0xEF < ch0
              &&   src_it != src_end && is_utf8_continuation(ch1 = * src_it)
              && first_2_of_4_are_valid(ch0, ch1)
              && ++src_it != src_end && is_utf8_continuation(*src_it)
              && ++src_it != src_end && is_utf8_continuation(*src_it) )
        {
            size += 4;
            ++src_it;
        }
        else
        {
            size += 3;
        }
    }
    return size;
}

STRF_STATIC_LINKAGE STRF_HD std::size_t utf8_codepoints_count
        ( const std::uint8_t* begin
        , const std::uint8_t* end
        , std::size_t max_count )
{
    std::size_t count = 0;
    for(auto it = begin; it != end && count < max_count; ++it)
    {
        if (!is_utf8_continuation(*it))
        {
            ++ count;
        }
    }
    return count;
}

STRF_STATIC_LINKAGE STRF_HD void utf8_encode_fill
    ( strf::underlying_outbuf<1>& ob
    , std::size_t count
    , char32_t ch
    , strf::encoding_error err_hdl
    , strf::surrogate_policy allow_surr )
{
    if (ch < 0x80)
    {
        strf::detail::write_fill( ob, count
                                         , static_cast<std::uint8_t>(ch) );
    }
    else if (ch < 0x800)
    {
        strf::detail::simple_array<std::uint8_t, 2> seq = {
            static_cast<std::uint8_t>(0xC0 | ((ch & 0x7C0) >> 6)),
            static_cast<std::uint8_t>(0x80 |  (ch &  0x3F))
        };
        strf::detail::repeat_sequence(ob, count, seq);
    }
    else if (ch <  0x10000)
    {
        if ( allow_surr == strf::surrogate_policy::strict
          && detail::is_surrogate(ch) )
        {
            goto invalid_char;
        }
        strf::detail::simple_array<std::uint8_t, 3> seq = {
            static_cast<std::uint8_t>(0xE0 | ((ch & 0xF000) >> 12)),
            static_cast<std::uint8_t>(0x80 | ((ch &  0xFC0) >> 6)),
            static_cast<std::uint8_t>(0x80 |  (ch &   0x3F)),
        };
        strf::detail::repeat_sequence(ob, count, seq);
    }
    else if (ch < 0x110000)
    {
        strf::detail::simple_array<std::uint8_t, 4> seq = {
            static_cast<std::uint8_t>(0xF0 | ((ch & 0x1C0000) >> 18)),
            static_cast<std::uint8_t>(0x80 | ((ch &  0x3F000) >> 12)),
            static_cast<std::uint8_t>(0x80 | ((ch &    0xFC0) >> 6)),
            static_cast<std::uint8_t>(0x80 |  (ch &     0x3F))
        };
        strf::detail::repeat_sequence(ob, count, seq);
    }
    else
    {
        invalid_char:
        switch(err_hdl)
        {
            case strf::encoding_error::stop:
                strf::detail::throw_encoding_failure();
                return;

            default:
            {
                STRF_ASSERT(err_hdl == strf::encoding_error::replace);
                strf::detail::simple_array<std::uint8_t, 3> seq
                    { 0xEF, 0xBF, 0xBD };
                strf::detail::repeat_sequence(ob, count, seq);
            }
        }
    }
}


STRF_STATIC_LINKAGE STRF_HD std::uint8_t* utf8_encode_char
    ( std::uint8_t* dest
    , char32_t ch )
{
    if (ch < 0x80)
    {
        *dest = static_cast<std::uint8_t>(ch);
        return dest + 1;
    }
    if (ch < 0x800)
    {
        dest[0] = static_cast<std::uint8_t>(0xC0 | ((ch & 0x7C0) >> 6));
        dest[1] = static_cast<std::uint8_t>(0x80 |  (ch &  0x3F));
        return dest + 2;
    }
    if (ch <  0x10000)
    {
        dest[0] = static_cast<std::uint8_t>(0xE0 | ((ch & 0xF000) >> 12));
        dest[1] = static_cast<std::uint8_t>(0x80 | ((ch &  0xFC0) >> 6));
        dest[2] = static_cast<std::uint8_t>(0x80 |  (ch &   0x3F));
        return dest + 3;
    }
    if (ch < 0x110000)
    {
        dest[0] = static_cast<std::uint8_t>(0xF0 | ((ch & 0x1C0000) >> 18));
        dest[1] = static_cast<std::uint8_t>(0x80 | ((ch &  0x3F000) >> 12));
        dest[2] = static_cast<std::uint8_t>(0x80 | ((ch &    0xFC0) >> 6));
        dest[3] = static_cast<std::uint8_t>(0x80 |  (ch &     0x3F));
        return dest + 4;
    }
    dest[0] = 0xEF;
    dest[1] = 0xBF;
    dest[2] = 0xBD;
    return dest + 3;
}

STRF_STATIC_LINKAGE STRF_HD void utf32_to_utf8_transcode
    ( strf::underlying_outbuf<1>& ob
    , const char32_t* src
    , const char32_t* src_end
    , strf::encoding_error err_hdl
    , strf::surrogate_policy allow_surr )
{
    auto src_it = src;
    auto dest_it = ob.pos();
    auto dest_end = ob.end();
    for(;src_it != src_end; ++src_it)
    {
        auto ch = *src_it;
        if(ch < 0x80)
        {
            STRF_CHECK_DEST;
            *dest_it = static_cast<std::uint8_t>(ch);
            ++dest_it;
        }
        else if (ch < 0x800)
        {
            STRF_CHECK_DEST_SIZE(2);
            dest_it[0] = static_cast<std::uint8_t>(0xC0 | ((ch & 0x7C0) >> 6));
            dest_it[1] = static_cast<std::uint8_t>(0x80 |  (ch &  0x3F));
            dest_it += 2;
        }
        else if (ch < 0x10000)
        {
            if ( allow_surr == strf::surrogate_policy::lax
              || strf::detail::not_surrogate(ch))
            {
                STRF_CHECK_DEST_SIZE(3);
                dest_it[0] = static_cast<std::uint8_t>(0xE0 | ((ch & 0xF000) >> 12));
                dest_it[1] = static_cast<std::uint8_t>(0x80 | ((ch &  0xFC0) >> 6));
                dest_it[2] = static_cast<std::uint8_t>(0x80 |  (ch &   0x3F));
                dest_it += 3;
            }
            else goto invalid_sequence;
        }
        else if (ch < 0x110000)
        {
            STRF_CHECK_DEST_SIZE(4);
            dest_it[0] = static_cast<std::uint8_t>(0xF0 | ((ch & 0x1C0000) >> 18));
            dest_it[1] = static_cast<std::uint8_t>(0x80 | ((ch &  0x3F000) >> 12));
            dest_it[2] = static_cast<std::uint8_t>(0x80 | ((ch &    0xFC0) >> 6));
            dest_it[3] = static_cast<std::uint8_t>(0x80 |  (ch &     0x3F));
            dest_it += 4;
        }
        else
        {
            invalid_sequence:
            switch (err_hdl)
            {
                case strf::encoding_error::replace:
                    STRF_CHECK_DEST_SIZE(3);
                    dest_it[0] = 0xEF;
                    dest_it[1] = 0xBF;
                    dest_it[2] = 0xBD;
                    dest_it += 3;
                    break;

                default:
                    STRF_ASSERT(err_hdl == strf::encoding_error::stop);
                    ob.advance_to(dest_it);
                    strf::detail::throw_encoding_failure();
            }
        }
    }
    ob.advance_to(dest_it);
}

STRF_STATIC_LINKAGE STRF_HD std::size_t utf32_to_utf8_size
    ( const char32_t* src
    , const char32_t* src_end
    , strf::surrogate_policy allow_surr )
{
    (void) allow_surr;
    auto src_it = src;
    std::size_t count = 0;
    for(;src_it != src_end; ++src_it)
    {
        auto ch = *src_it;
        if (ch < 0x110000)
        {
            count += 1 + (ch >= 0x80) + (ch >= 0x800) + (ch >= 0x10000);
        }
        else
        {
            count += 3;
        }
    }
    return count;
}

STRF_STATIC_LINKAGE STRF_HD void utf8_write_replacement_char
    ( strf::underlying_outbuf<1>& ob )
{
    auto dest_it = ob.pos();
    auto dest_end = ob.end();
    STRF_CHECK_DEST_SIZE(3);
    dest_it[0] = 0xEF;
    dest_it[1] = 0xBF;
    dest_it[2] = 0xBD;
    dest_it += 3;
    ob.advance_to(dest_it);
}

STRF_STATIC_LINKAGE STRF_HD char32_t utf8_decode_single_char(std::uint8_t ch)
{
    const std::uint8_t uch = ch;
    return uch < 0x80 ? static_cast<char32_t>(uch) : static_cast<char32_t>(-1);
}

STRF_STATIC_LINKAGE STRF_HD std::size_t utf8_validate(char32_t ch)
{
    return ( ch < 0x80     ? 1 :
             ch < 0x800    ? 2 :
             ch < 0x10000  ? 3 :
             ch < 0x110000 ? 4 : (std::size_t)-1 );
}

STRF_STATIC_LINKAGE STRF_HD void utf16_to_utf32_transcode
    ( strf::underlying_outbuf<4>& ob
    , const char16_t* src
    , const char16_t* src_end
    , strf::encoding_error err_hdl
    , strf::surrogate_policy allow_surr )
{
    unsigned long ch, ch2;
    char32_t ch32;
    const char16_t* src_it_next;
    auto dest_it = ob.pos();
    auto dest_end = ob.end();
    for(auto src_it = src; src_it != src_end; src_it = src_it_next)
    {
        src_it_next = src_it + 1;
        ch = *src_it;
        src_it_next = src_it + 1;

        if (not_surrogate(ch))
        {
            ch32 = ch;
        }
        else if ( is_high_surrogate(ch)
               && src_it_next != src_end
               && is_low_surrogate(ch2 = *src_it_next))
        {
            ch32 = 0x10000 + (((ch & 0x3FF) << 10) | (ch2 & 0x3FF));
            ++src_it_next;
        }
        else if (allow_surr == strf::surrogate_policy::lax)
        {
            ch32 = ch;
        }
        else
        {
            ch32 = 0xFFFD;
            if (err_hdl == strf::encoding_error::stop)
            {
                ob.advance_to(dest_it);
                strf::detail::throw_encoding_failure();
            }
        }

        STRF_CHECK_DEST;
        *dest_it = ch32;
        ++dest_it;
    }
    ob.advance_to(dest_it);
}

STRF_STATIC_LINKAGE STRF_HD std::size_t utf16_to_utf32_size
    ( const char16_t* src
    , const char16_t* src_end
    , strf::surrogate_policy allow_surr )
{
    (void) allow_surr;
    unsigned long ch;
    std::size_t count = 0;
    const char16_t* src_it = src;
    const char16_t* src_it_next;
    for(; src_it != src_end; src_it = src_it_next)
    {
        src_it_next = src_it + 1;
        ch = *src_it;
        src_it_next = src_it + 1;

        ++count;
        if ( is_high_surrogate(ch)
          && src_it_next != src_end
          && is_low_surrogate(*src_it_next))
        {
            ++src_it_next;
        }
    }
    return count;
}

STRF_STATIC_LINKAGE STRF_HD void utf16_sanitize
    ( strf::underlying_outbuf<2>& ob
    , const char16_t* src
    , const char16_t* src_end
    , strf::encoding_error err_hdl
    , strf::surrogate_policy allow_surr )
{
    unsigned long ch, ch2;
    auto src_it = src;
    const char16_t* src_it_next;
    auto dest_it = ob.pos();
    auto dest_end = ob.end();
    for( ; src_it != src_end; src_it = src_it_next)
    {
        ch = *src_it;
        src_it_next = src_it + 1;

        if (not_surrogate(ch))
        {
            STRF_CHECK_DEST;
            *dest_it = static_cast<char16_t>(ch);
            ++dest_it;
        }
        else if ( is_high_surrogate(ch)
               && src_it_next != src_end
               && is_low_surrogate(ch2 = *src_it_next))
        {
            ++src_it_next;
            STRF_CHECK_DEST_SIZE(2);
            dest_it[0] = static_cast<char16_t>(ch);
            dest_it[1] = static_cast<char16_t>(ch2);
            dest_it += 2;
        }
        else if (allow_surr == strf::surrogate_policy::lax)
        {
            STRF_CHECK_DEST;
            *dest_it = static_cast<char16_t>(ch);
            ++dest_it;
        }
        else
        {
            if (err_hdl == strf::encoding_error::stop)
            {
                ob.advance_to(dest_it);
                strf::detail::throw_encoding_failure();
            }
            STRF_CHECK_DEST;
            *dest_it = 0xFFFD;
            ++dest_it;
        }
    }
    ob.advance_to(dest_it);
}

STRF_STATIC_LINKAGE STRF_HD std::size_t utf16_sanitize_size
    ( const char16_t* src
    , const char16_t* src_end
    , strf::surrogate_policy allow_surr )
{
    (void) allow_surr;
    std::size_t count = 0;
    const char16_t* src_it = src;
    unsigned long ch;
    while (src_it != src_end)
    {
        ch = *src_it;
        ++ src_it;
        ++ count;
        if ( is_high_surrogate(ch)
          && src_it != src_end
          && is_low_surrogate(*src_it))
        {
            ++ src_it;
            ++ count;
        }
    }
    return count;
}

STRF_STATIC_LINKAGE STRF_HD std::size_t utf16_codepoints_count
    ( const char16_t* begin
    , const char16_t* end
    , std::size_t max_count )
{
    std::size_t count = 0;
    for(auto it = begin; it != end && count < max_count; ++it, ++count)
    {
        if(is_high_surrogate(*it))
        {
            ++it;
        }
    }
    return count;
}

STRF_STATIC_LINKAGE STRF_HD std::size_t utf16_validate(char32_t ch)
{
    return ch < 0x10000 ? 1 : ch < 0x110000 ? 2 : (std::size_t)-1;
}

STRF_STATIC_LINKAGE STRF_HD char16_t* utf16_encode_char
    ( char16_t* dest
    , char32_t ch )
{
    if (ch < 0x10000)
    {
        *dest = static_cast<char16_t>(ch);
        return dest + 1;
    }
    if (ch < 0x110000)
    {
        char32_t sub_codepoint = ch - 0x10000;
        dest[0] = static_cast<char16_t>(0xD800 + ((sub_codepoint & 0xFFC00) >> 10));
        dest[1] = static_cast<char16_t>(0xDC00 +  (sub_codepoint &  0x3FF));
        return dest + 2;
    }
    *dest = 0xFFFD;
    return dest + 1;
}

STRF_STATIC_LINKAGE STRF_HD void utf16_encode_fill
    ( strf::underlying_outbuf<2>& ob
    , std::size_t count
    , char32_t ch
    , strf::encoding_error err_hdl
    , strf::surrogate_policy allow_surr )
{
    if (ch < 0x10000)
    {
        if ( allow_surr  == strf::surrogate_policy::strict
          && detail::is_surrogate(ch) )
        {
            goto invalid_char;
        }
        strf::detail::write_fill(ob, count, static_cast<char16_t>(ch));
    }
    else if (ch < 0x110000)
    {
        char32_t sub_codepoint = ch - 0x10000;
        strf::detail::simple_array<char16_t, 2> seq = {
            static_cast<char16_t>(0xD800 + ((sub_codepoint & 0xFFC00) >> 10)),
            static_cast<char16_t>(0xDC00 +  (sub_codepoint &  0x3FF))
        };
        strf::detail::repeat_sequence(ob, count, seq);
    }
    else
    {
        invalid_char:
        if (err_hdl == strf::encoding_error::stop)
        {
            strf::detail::throw_encoding_failure();
        }
        strf::detail::write_fill(ob, count, u'\uFFFD');
    }
}

STRF_STATIC_LINKAGE STRF_HD void utf32_to_utf16_transcode
    ( strf::underlying_outbuf<2>& ob
    , const char32_t* src
    , const char32_t* src_end
    , strf::encoding_error err_hdl
    , strf::surrogate_policy allow_surr )
{
    auto src_it = src;
    auto dest_it = ob.pos();
    auto dest_end = ob.end();
    for ( ; src_it != src_end; ++src_it)
    {
        auto ch = *src_it;
        if (ch < 0x10000)
        {
            if ( allow_surr == strf::surrogate_policy::lax
              || strf::detail::not_surrogate(ch) )
            {
                STRF_CHECK_DEST;
                *dest_it = static_cast<char16_t>(ch);
                ++dest_it;
            }
            else goto invalid_char;
        }
        else if (ch < 0x110000)
        {
            STRF_CHECK_DEST_SIZE(2);
            char32_t sub_codepoint = ch - 0x10000;
            dest_it[0] = static_cast<char16_t>(0xD800 + ((sub_codepoint & 0xFFC00) >> 10));
            dest_it[1] = static_cast<char16_t>(0xDC00 +  (sub_codepoint &  0x3FF));
            dest_it += 2;
        }
        else
        {
            invalid_char:
            if (err_hdl == strf::encoding_error::stop)
            {
                ob.advance_to(dest_it);
                strf::detail::throw_encoding_failure();
            }
            STRF_CHECK_DEST;
            *dest_it = 0xFFFD;
            ++dest_it;
        }
    }
    ob.advance_to(dest_it);
}

STRF_STATIC_LINKAGE STRF_HD std::size_t utf32_to_utf16_size
    ( const char32_t* src
    , const char32_t* src_end
    , strf::surrogate_policy allow_surr )
{
    (void) allow_surr;
    std::size_t count = 0;
    const char32_t* src_it = src;
    for ( ; src_it != src_end; ++src_it)
    {
        auto ch = *src_it;
        count += 1 + (0x10000 <= ch && ch < 0x110000);
    }
    return count;
}

STRF_STATIC_LINKAGE STRF_HD void utf16_write_replacement_char
    ( strf::underlying_outbuf<2>& ob )
{
    ob.ensure(1);
    *ob.pos() = 0xFFFD;
    ob.advance();
}

STRF_STATIC_LINKAGE STRF_HD std::size_t utf32_sanitize_size
    ( const char32_t* src
    , const char32_t* src_end
    , strf::surrogate_policy allow_surr )
{
    (void) allow_surr;
    return src_end - src;
}

STRF_STATIC_LINKAGE STRF_HD void utf32_sanitize
    ( strf::underlying_outbuf<4>& ob
    , const char32_t* src
    , const char32_t* src_end
    , strf::encoding_error err_hdl
    , strf::surrogate_policy allow_surr )
{
    auto dest_it = ob.pos();
    auto dest_end = ob.end();
    if (allow_surr == strf::surrogate_policy::lax)
    {
        for (auto src_it = src; src_it < src_end; ++src_it)
        {
            auto ch = *src_it;
            if (ch >= 0x110000)
            {
                if (err_hdl == strf::encoding_error::stop)
                {
                    ob.advance_to(dest_it);
                    strf::detail::throw_encoding_failure();
                }
                ch = 0xFFFD;
            }
            STRF_CHECK_DEST;
            *dest_it = ch;
            ++dest_it;
        }
    }
    else
    {
        for(auto src_it = src; src_it < src_end; ++src_it)
        {
            char32_t ch = *src_it;
            if (ch >= 0x110000 || strf::detail::is_surrogate(ch))
            {
                if (err_hdl == strf::encoding_error::stop)
                {
                    ob.advance_to(dest_it);
                    strf::detail::throw_encoding_failure();
                }
                ch = 0xFFFD;
            }
            STRF_CHECK_DEST;
            *dest_it = ch;
            ++dest_it;
        }
    }
    ob.advance_to(dest_it);
}

inline STRF_HD std::size_t utf32_codepoints_count
    ( const char32_t* begin
    , const char32_t* end
    , std::size_t max_count )
{
    std::size_t len = end - begin;
    return len < max_count ? len : max_count;
}

inline STRF_HD std::size_t utf32_validate(char32_t ch)
{
    (void)ch;
    return 1;
}

STRF_STATIC_LINKAGE STRF_HD char32_t* utf32_encode_char
    ( char32_t* dest
    , char32_t ch )
{
    *dest = ch;
    return dest + 1;
}

STRF_STATIC_LINKAGE STRF_HD void utf32_encode_fill
    ( strf::underlying_outbuf<4>& ob
    , std::size_t count
    , char32_t ch
    , strf::encoding_error err_hdl
    , strf::surrogate_policy allow_surr )
{
    if (ch > 0x10FFFF || ( allow_surr == strf::surrogate_policy::strict
                        && detail::is_surrogate(ch) ))
    {
        if (err_hdl == strf::encoding_error::stop)
        {
            strf::detail::throw_encoding_failure();
        }
        STRF_ASSERT(err_hdl == strf::encoding_error::replace);
        ch = 0xFFFD;
    }
    strf::detail::write_fill(ob, count, ch);
}

STRF_STATIC_LINKAGE STRF_HD void utf32_write_replacement_char
    ( strf::underlying_outbuf<4>& ob )
{
    ob.ensure(1);
    *ob.pos() = 0xFFFD;
    ob.advance();
}

inline STRF_HD char32_t utf16_decode_single_char(char16_t ch)
{
    return ch;
}

inline STRF_HD char32_t utf32_decode_single_char(char32_t ch)
{
    return ch;
}

STRF_STATIC_LINKAGE STRF_HD void utf8_to_utf16_transcode
    ( strf::underlying_outbuf<2>& ob
    , const std::uint8_t* src
    , const std::uint8_t* src_end
    , strf::encoding_error err_hdl
    , strf::surrogate_policy allow_surr )
{
    using strf::detail::utf8_decode;

    std::uint8_t ch0, ch1, ch2, ch3;
    unsigned long x;
    auto src_it = src;
    auto dest_it = ob.pos();
    auto dest_end = ob.end();

    for (;src_it != src_end; ++dest_it)
    {
        ch0 = (*src_it);
        ++src_it;
        if (ch0 < 0x80)
        {
            STRF_CHECK_DEST;
            *dest_it = ch0;
        }
        else if (0xC0 == (ch0 & 0xE0))
        {
            if ( ch0 > 0xC1
              && src_it != src_end && is_utf8_continuation(ch1 = * src_it))
            {
                STRF_CHECK_DEST;
                *dest_it = utf8_decode(ch0, ch1);
                ++src_it;
            } else goto invalid_sequence;
        }
        else if (0xE0 == ch0)
        {
            if (   src_it != src_end && (((ch1 = * src_it) & 0xE0) == 0xA0)
              && ++src_it != src_end && is_utf8_continuation(ch2 = * src_it) )
            {
                STRF_CHECK_DEST;
                *dest_it = ((ch1 & 0x3F) << 6) | (ch2 & 0x3F);
                ++src_it;
            } else goto invalid_sequence;
        }
        else if (0xE0 == (ch0 & 0xF0))
        {
            if (   src_it != src_end && is_utf8_continuation(ch1 = * src_it)
              && first_2_of_3_are_valid( x = utf8_decode_first_2_of_3(ch0, ch1)
                                       , allow_surr )
              && ++src_it != src_end && is_utf8_continuation(ch2 = * src_it) )
            {
                STRF_CHECK_DEST;
                *dest_it = static_cast<char16_t>((x << 6) | (ch2 & 0x3F));
                ++src_it;
            } else goto invalid_sequence;
        }
        else if (0xEF < ch0)
        {
            if ( src_it != src_end && is_utf8_continuation(ch1 = * src_it)
                 && first_2_of_4_are_valid(x = utf8_decode_first_2_of_4(ch0, ch1))
                 && ++src_it != src_end && is_utf8_continuation(ch2 = * src_it)
                 && ++src_it != src_end && is_utf8_continuation(ch3 = * src_it) )
            {
                STRF_CHECK_DEST_SIZE(2);
                x = utf8_decode_last_2_of_4(x, ch2, ch3) - 0x10000;
                dest_it[0] = static_cast<char16_t>(0xD800 + ((x & 0xFFC00) >> 10));
                dest_it[1] = static_cast<char16_t>(0xDC00 +  (x & 0x3FF));
                ++dest_it;
                ++src_it;
            } else goto invalid_sequence;
        }
        else
        {
            invalid_sequence:
            if (err_hdl == strf::encoding_error::stop)
            {
                ob.advance_to(dest_it);
                strf::detail::throw_encoding_failure();
            }
            STRF_CHECK_DEST;
            *dest_it = 0xFFFD;
        }
    }
    ob.advance_to(dest_it);
}

STRF_STATIC_LINKAGE STRF_HD std::size_t utf8_to_utf16_size
    ( const std::uint8_t* src_begin
    , const std::uint8_t* src_end
    , strf::surrogate_policy allow_surr )
{
    using strf::detail::utf8_decode;
    using strf::detail::not_surrogate;

    std::size_t size = 0;
    std::uint8_t ch0, ch1;
    auto src_it = src_begin;
    while(src_it < src_end)
    {
        ch0 = *src_it;
        ++src_it;
        ++size;
        if (0xC0 == (ch0 & 0xE0))
        {
            if (ch0 > 0xC1 && src_it != src_end && is_utf8_continuation(*src_it))
            {
                ++src_it;
            }
        }
        else if (0xE0 == ch0)
        {
            if (   src_it != src_end && (((ch1 = * src_it) & 0xE0) == 0xA0)
              && ++src_it != src_end && is_utf8_continuation(* src_it) )
            {
                ++src_it;
            }
        }
        else if (0xE0 == (ch0 & 0xF0))
        {
            if ( src_it != src_end && is_utf8_continuation(ch1 = * src_it)
              && first_2_of_3_are_valid( ch0, ch1, allow_surr )
              && ++src_it != src_end && is_utf8_continuation(* src_it) )
            {
                ++src_it;
            }
        }
        else if(0xEF < ch0)
        {
            if (   src_it != src_end && is_utf8_continuation(ch1 = * src_it)
              && first_2_of_4_are_valid(ch0, ch1)
              && ++src_it != src_end && is_utf8_continuation(*src_it)
              && ++src_it != src_end && is_utf8_continuation(*src_it) )
            {
                ++src_it;
                ++size;
            }
        }

    }
    return size;
}

STRF_STATIC_LINKAGE STRF_HD void utf16_to_utf8_transcode
    ( strf::underlying_outbuf<1>& ob
    , const char16_t* src
    , const char16_t* src_end
    , strf::encoding_error err_hdl
    , strf::surrogate_policy allow_surr )
{
    (void) err_hdl;
    auto src_it = src;
    auto dest_it = ob.pos();
    auto dest_end = ob.end();

    for( ; src_it < src_end; ++src_it)
    {
        auto ch = *src_it;
        if (ch < 0x80)
        {
            STRF_CHECK_DEST;
            *dest_it = static_cast<std::uint8_t>(ch);
            ++dest_it;
        }
        else if (ch < 0x800)
        {
            STRF_CHECK_DEST_SIZE(2);
            dest_it[0] = static_cast<std::uint8_t>(0xC0 | ((ch & 0x7C0) >> 6));
            dest_it[1] = static_cast<std::uint8_t>(0x80 |  (ch &  0x3F));
            dest_it += 2;
        }
        else if (not_surrogate(ch))
        {
            three_bytes:
            STRF_CHECK_DEST_SIZE(3);
            dest_it[0] = static_cast<std::uint8_t>(0xE0 | ((ch & 0xF000) >> 12));
            dest_it[1] = static_cast<std::uint8_t>(0x80 | ((ch &  0xFC0) >> 6));
            dest_it[2] = static_cast<std::uint8_t>(0x80 |  (ch &   0x3F));
            dest_it += 3;
        }
        else if ( strf::detail::is_high_surrogate(ch)
               && src_it != src_end
               && strf::detail::is_low_surrogate(*(src_it + 1)))
        {
            STRF_CHECK_DEST_SIZE(4);
            unsigned long ch2 = *++src_it;
            unsigned long codepoint = 0x10000 + (((ch & 0x3FF) << 10) | (ch2 & 0x3FF));
            dest_it[0] = static_cast<std::uint8_t>(0xF0 | ((codepoint & 0x1C0000) >> 18));
            dest_it[1] = static_cast<std::uint8_t>(0x80 | ((codepoint &  0x3F000) >> 12));
            dest_it[2] = static_cast<std::uint8_t>(0x80 | ((codepoint &    0xFC0) >> 6));
            dest_it[3] = static_cast<std::uint8_t>(0x80 |  (codepoint &     0x3F));
            dest_it += 4;
        }
        else if (allow_surr == strf::surrogate_policy::lax)
        {
            goto three_bytes;
        }
        else // invalid sequece
        {
            if (err_hdl == strf::encoding_error::stop)
            {
                ob.advance_to(dest_it);
                strf::detail::throw_encoding_failure();
            }
            STRF_CHECK_DEST_SIZE(3);
            dest_it[0] = 0xEF;
            dest_it[1] = 0xBF;
            dest_it[2] = 0xBD;
            dest_it += 3;
        }
    }
    ob.advance_to(dest_it);
}

STRF_STATIC_LINKAGE STRF_HD std::size_t utf16_to_utf8_size
    ( const char16_t* src_begin
    , const char16_t* src_end
    , strf::surrogate_policy allow_surr )
{
    (void) allow_surr;
    std::size_t size = 0;
    for(auto it = src_begin; it < src_end; ++it)
    {
        char16_t ch = *it;
        if (ch < 0x80)
        {
            ++size;
        }
        else if (ch < 0x800)
        {
            size += 2;
        }
        else if ( strf::detail::is_high_surrogate(ch)
               && it + 1 != src_end
               && strf::detail::is_low_surrogate(*(it + 1)) )
        {
            size += 4;
            ++it;
        }
        else
        {
            size += 3;
        }
    }
    return size;
}

STRF_STATIC_LINKAGE
STRF_HD const strf::detail::transcoder_impl<std::uint8_t, char16_t>* utf8_to_enc16
    ( const strf::detail::encoding_impl<char16_t>& other )
{
    if (other.id == encoding_id::eid_utf16)
    {
        static const strf::detail::transcoder_impl<std::uint8_t, char16_t> tr_obj =
            { strf::detail::utf8_to_utf16_transcode
            , strf::detail::utf8_to_utf16_size };
        return & tr_obj;
    }
    return nullptr;
}

STRF_STATIC_LINKAGE
STRF_HD const strf::detail::transcoder_impl<char16_t, std::uint8_t>* utf8_from_enc16
    ( const strf::detail::encoding_impl<char16_t>& other )
{
    if (other.id == encoding_id::eid_utf16)
    {
        static const strf::detail::transcoder_impl<char16_t, std::uint8_t> tr_obj =
            { strf::detail::utf16_to_utf8_transcode
            , strf::detail::utf16_to_utf8_size };
        return & tr_obj;
    }
    return nullptr;
}

STRF_INLINE
STRF_HD const strf::detail::encoding_impl<std::uint8_t>& utf8_impl()
{
    static const strf::detail::encoding_impl<std::uint8_t> encoding_obj =
         { { strf::detail::utf32_to_utf8_transcode
           , strf::detail::utf32_to_utf8_size }
         , { strf::detail::utf8_to_utf32_transcode
           , strf::detail::utf8_to_utf32_size }
         , { strf::detail::utf8_sanitize
           , strf::detail::utf8_sanitize_size }
         , strf::detail::utf8_validate
         , strf::detail::utf8_encode_char
         , strf::detail::utf8_encode_fill
         , strf::detail::utf8_codepoints_count
         , strf::detail::utf8_write_replacement_char
         , strf::detail::utf8_decode_single_char
         , nullptr, nullptr
         , strf::detail::utf8_from_enc16
         , strf::detail::utf8_to_enc16
         , nullptr, nullptr
         , "UTF-8"
         , strf::encoding_id::eid_utf8
         , 3, 0, 0x80 };

    return encoding_obj;
}

STRF_INLINE
STRF_HD const strf::detail::encoding_impl<char16_t>& utf16_impl()
{
    static const strf::detail::encoding_impl<char16_t> encoding_obj =
        { { strf::detail::utf32_to_utf16_transcode
          , strf::detail::utf32_to_utf16_size }
        , { strf::detail::utf16_to_utf32_transcode
          , strf::detail::utf16_to_utf32_size }
        , { strf::detail::utf16_sanitize
          , strf::detail::utf16_sanitize_size }
        , strf::detail::utf16_validate
        , strf::detail::utf16_encode_char
        , strf::detail::utf16_encode_fill
        , strf::detail::utf16_codepoints_count
        , strf::detail::utf16_write_replacement_char
        , strf::detail::utf16_decode_single_char
        , nullptr, nullptr, nullptr, nullptr, nullptr, nullptr
        , "UTF-16"
        , strf::encoding_id::eid_utf16
        , 1, 0, 0xFFFF };

    return encoding_obj;
}

STRF_INLINE
STRF_HD const  strf::detail::encoding_impl<char32_t>& utf32_impl()
{
    static const strf::detail::encoding_impl<char32_t> encoding_obj =
        { { strf::detail::utf32_sanitize
          , strf::detail::utf32_sanitize_size }
        , { strf::detail::utf32_sanitize
          , strf::detail::utf32_sanitize_size }
        , { strf::detail::utf32_sanitize
          , strf::detail::utf32_sanitize_size }
        , strf::detail::utf32_validate
        , strf::detail::utf32_encode_char
        , strf::detail::utf32_encode_fill
        , strf::detail::utf32_codepoints_count
        , strf::detail::utf32_write_replacement_char
        , strf::detail::utf32_decode_single_char
        , nullptr, nullptr, nullptr, nullptr, nullptr, nullptr
        , "UTF-32"
        , strf::encoding_id::eid_utf32
        , 1, 0, 0x10FFFF };

    return encoding_obj;
}

} // namespace detail

STRF_NAMESPACE_END

#endif  // STRF_DETAIL_UTF_ENCODINGS_HPP

