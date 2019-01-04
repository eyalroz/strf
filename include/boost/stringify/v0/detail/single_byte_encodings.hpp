#ifndef BOOST_STRINGIFY_V0_DETAIL_SINGLE_BYTE_ENCODINGS_HPP
#define BOOST_STRINGIFY_V0_DETAIL_SINGLE_BYTE_ENCODINGS_HPP

//  Distributed under the Boost Software License, Version 1.0.
//  (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)

BOOST_STRINGIFY_V0_NAMESPACE_BEGIN
namespace detail {

template<size_t SIZE, class T>
constexpr size_t array_size(T (&)[SIZE]) {
    return SIZE;
}

struct ch32_to_char
{
    char32_t key;
    unsigned value;
};

struct cmp_ch32_to_char
{
    bool operator()(ch32_to_char a, ch32_to_char b) const
    {
        return a.key < b.key;
    }
};

template <typename CharIn>
static std::size_t same_size
    ( const CharIn* src
    , const CharIn* src_end
    , stringify::v0::error_handling err_hdl
    , bool allow_surr )
{
    (void) allow_surr;
    (void) err_hdl;
    return src_end - src;
}


template <class Impl>
struct single_byte_encoding
{
    static stringify::v0::cv_result to_utf32
        ( const std::uint8_t** src
        , const std::uint8_t* src_end
        , char32_t** dest
        , char32_t* dest_end
        , stringify::v0::error_handling err_hdl
        , bool allow_surr );

    static stringify::v0::cv_result from_utf32
        ( const char32_t** src
        , const char32_t* src_end
        , std::uint8_t** dest
        , std::uint8_t* dest_end
        , stringify::v0::error_handling err_hdl
        , bool allow_surr );

    static stringify::v0::cv_result sanitize
        ( const std::uint8_t** src
        , const std::uint8_t* src_end
        , std::uint8_t** dest
        , std::uint8_t* dest_end
        , stringify::v0::error_handling err_hdl
        , bool allow_surr );

    static stringify::v0::cv_result encode_char
        ( std::uint8_t** dest
        , std::uint8_t* end
        , char32_t ch
        , stringify::v0::error_handling err_hdl );

    static stringify::v0::cv_result encode_fill
        ( std::uint8_t** dest
        , std::uint8_t* end
        , std::size_t& count
        , char32_t ch
        , stringify::v0::error_handling err_hdl );

    static char32_t decode_single_char(std::uint8_t ch)
    {
        return Impl::decode(ch);
    }

    static std::size_t codepoints_count
        ( const std::uint8_t* begin
        , const std::uint8_t* end
        , std::size_t max_count );

    static std::size_t replacement_char_size();

    static bool write_replacement_char
        ( std::uint8_t** dest
        , std::uint8_t* dest_end );

    static std::size_t validate(char32_t ch);
};


template <class Impl>
std::size_t single_byte_encoding<Impl>::codepoints_count
    ( const std::uint8_t* begin
    , const std::uint8_t* end
    , std::size_t max_count )
{
    std::size_t len = end - begin;
    return len < max_count ? len : max_count;
}

template <class Impl>
stringify::v0::cv_result single_byte_encoding<Impl>::to_utf32
    ( const std::uint8_t** src
    , const std::uint8_t* src_end
    , char32_t** dest
    , char32_t* dest_end
    , stringify::v0::error_handling err_hdl
    , bool allow_surr )
{
    (void) allow_surr;
    auto dest_it = *dest;
    for (auto src_it = *src; src_it < src_end; ++src_it)
    {
        if(dest_it != dest_end)
        {
            char32_t ch32 = Impl::decode(*src_it);
            if(ch32 != (char32_t)-1)
            {
                *dest_it = ch32;
                ++dest_it;
            }
            else
            {
                switch(err_hdl)
                {
                    case stringify::v0::error_handling::stop:
                        *src = src_it + 1;
                        *dest = dest_it;
                        return stringify::v0::cv_result::invalid_char;
                    case stringify::v0::error_handling::replace:
                        *dest_it = 0xFFFD;
                        ++dest_it;
                        break;
                    default:
                        BOOST_ASSERT(err_hdl == stringify::v0::error_handling::ignore);
                        break;
                }
            }
        }
        else
        {
            *src = src_it;
            *dest = dest_end;
            return stringify::v0::cv_result::insufficient_space;
        }
    }
    *src = src_end;
    *dest = dest_it;
    return stringify::v0::cv_result::success;
}

template <class Impl>
stringify::v0::cv_result single_byte_encoding<Impl>::sanitize
    ( const std::uint8_t** src
    , const std::uint8_t* src_end
    , std::uint8_t** dest
    , std::uint8_t* dest_end
    , stringify::v0::error_handling err_hdl
    , bool allow_surr )
{
    (void) allow_surr;
    auto dest_it = *dest;
    for (auto src_it = *src; src_it < src_end; ++src_it)
    {
        if(dest_it != dest_end)
        {
            std::uint8_t ch = *src_it;
            if (Impl::is_valid(ch))
            {
                *dest_it = ch;
                ++dest_it;
            }
            else
            {
                switch(err_hdl)
                {
                    case stringify::v0::error_handling::stop:
                        *src = src_it + 1;
                        *dest = dest_it;
                        return stringify::v0::cv_result::invalid_char;
                    case stringify::v0::error_handling::replace:
                        *dest_it = '?';
                        ++dest_it;
                        break;
                    default:
                        BOOST_ASSERT(err_hdl == stringify::v0::error_handling::ignore);
                        break;
                }
            }
        }
        else
        {
            *src = src_it;
            *dest = dest_end;
            return stringify::v0::cv_result::insufficient_space;
        }
    }
    *src = src_end;
    *dest = dest_it;
    return stringify::v0::cv_result::success;
}


template <class Impl>
bool single_byte_encoding<Impl>::write_replacement_char
    ( std::uint8_t** dest
    , std::uint8_t* dest_end )
{
    auto dest_it = *dest;
    if (dest_it != dest_end)
    {
        *dest_it = '?';
        *dest = dest_it + 1;
        return true;
    }
    return false;
}

template <class Impl>
std::size_t single_byte_encoding<Impl>::replacement_char_size()
{
    return 1;
}

template <class Impl>
std::size_t single_byte_encoding<Impl>::validate(char32_t ch)
{
    return Impl::encode(ch) < 0x100 ? 1 : (std::size_t)-1;
}

template <class Impl>
stringify::v0::cv_result single_byte_encoding<Impl>::encode_char
    ( std::uint8_t** dest
    , std::uint8_t* end
    , char32_t ch
    , stringify::v0::error_handling err_hdl )
{
    std::uint8_t* dest_it = *dest;
    if(dest_it != end)
    {
        auto ch2 = Impl::encode(ch);
        if(ch2 < 0x100)
        {
            *dest_it = ch2;
            *dest = dest_it + 1;
            return stringify::v0::cv_result::success;
        }
        switch(err_hdl)
        {
            case stringify::v0::error_handling::stop:
                return stringify::v0::cv_result::invalid_char;
            case stringify::v0::error_handling::replace:
                *dest_it = '?';
                *dest = dest_it + 1;
                break;
            default:
                BOOST_ASSERT(err_hdl == stringify::v0::error_handling::ignore);
                break;
        }
        return stringify::v0::cv_result::success;
    }
    return stringify::v0::cv_result::insufficient_space;
}

template <class Impl>
stringify::v0::cv_result single_byte_encoding<Impl>::encode_fill
    ( std::uint8_t** dest
    , std::uint8_t* end
    , std::size_t& count
    , char32_t ch
    , stringify::v0::error_handling err_hdl )
{
    unsigned ch2 = Impl::encode(ch);

    if (ch2 < 0x100)
    {
        do_write:

        std::uint8_t* dest_it = *dest;
        std::size_t count_ = count;
        std::size_t available = end - dest_it;

        if (count_ <= available)
        {
            std::memset(dest, ch2, count_);
            count = 0;
            *dest = dest_it + count_;
            return stringify::v0::cv_result::success;
        }
        std::memset(dest, ch2, available);
        count = count_ - available;
        *dest = end;
        return stringify::v0::cv_result::insufficient_space;
    }
    switch(err_hdl)
    {
        case stringify::v0::error_handling::stop:
            return stringify::v0::cv_result::invalid_char;
        case stringify::v0::error_handling::replace:
            ch2 = '?';
            goto do_write;
        default:
            BOOST_ASSERT(err_hdl == stringify::v0::error_handling::ignore);
            break;
    }
    return stringify::v0::cv_result::success;
}

template <class Impl>
stringify::v0::cv_result single_byte_encoding<Impl>::from_utf32
    ( const char32_t** src
    , const char32_t* src_end
    , std::uint8_t** dest
    , std::uint8_t* dest_end
    , stringify::v0::error_handling err_hdl
    , bool allow_surr )
{
    (void)allow_surr;
    auto dest_it = *dest;
    auto src_it = *src;
    for(; src_it != src_end; ++src_it)
    {
        if(dest_it == dest_end)
        {
            *dest = dest_end;
            *src = src_it;
            return stringify::v0::cv_result::insufficient_space;
        }
        auto ch2 = Impl::encode(*src_it);
        if(ch2 < 0x100)
        {
            *dest_it = ch2;
            ++dest_it;
        }
        else
        {
            switch(err_hdl)
            {
                case stringify::v0::error_handling::stop:
                    *src = src_it + 1;
                    *dest = dest_it;
                    return stringify::v0::cv_result::invalid_char;

                case stringify::v0::error_handling::replace:
                    *dest_it = '?';
                    ++dest_it;
                    break;

                default:
                    BOOST_ASSERT(err_hdl == stringify::v0::error_handling::ignore);
                    break;
            }
        }
    }
    *src = src_end;
    *dest = dest_it;
    return stringify::v0::cv_result::success;
}

struct impl_strict_ascii
{
    static bool is_valid(std::uint8_t ch)
    {
        return ch < 0x80;
    }

    static char32_t decode(std::uint8_t ch)
    {
        if (ch < 0x80)
            return ch;
        return -1;
    }

    static unsigned encode(char32_t ch)
    {
        return ch < 0x80 ? ch : 0x100;
    }
};

struct impl_iso8859_1
{
    static bool is_valid(std::uint8_t)
    {
        return true;
    }

    static char32_t decode(std::uint8_t ch)
    {
        return ch;
    }

    static unsigned encode(char32_t ch)
    {
        return ch;
    }
};



struct impl_iso8859_3
{
    static bool is_valid(std::uint8_t ch)
    {
        return ch != 0xA5
            && ch != 0xAE
            && ch != 0xBE
            && ch != 0xC3
            && ch != 0xD0
            && ch != 0xE3
            && ch != 0xF0;
    }

    static unsigned encode(char32_t ch);

    static char32_t decode(std::uint8_t ch);
};

BOOST_STRINGIFY_INLINE unsigned impl_iso8859_3::encode(char32_t ch)
{
    if (ch < 0xA1)
    {
        return ch;
    }
    static const ch32_to_char enc_map[] =
        { {0x00A3, 0xA3}, {0x00A4, 0xA4}, {0x00A7, 0xA7}, {0x00A8, 0xA8}
        , {0x00AD, 0xAD}, {0x00B0, 0xB0}, {0x00B2, 0xB2}, {0x00B3, 0xB3}
        , {0x00B4, 0xB4}, {0x00B5, 0xB5}, {0x00B7, 0xB7}, {0x00B8, 0xB8}
        , {0x00BD, 0xBD}, {0x00C0, 0xC0}, {0x00C1, 0xC1}, {0x00C2, 0xC2}
        , {0x00C4, 0xC4}, {0x00C7, 0xC7}, {0x00C8, 0xC8}, {0x00C9, 0xC9}
        , {0x00CA, 0xCA}, {0x00CB, 0xCB}, {0x00CC, 0xCC}, {0x00CD, 0xCD}
        , {0x00CE, 0xCE}, {0x00CF, 0xCF}, {0x00D1, 0xD1}, {0x00D2, 0xD2}
        , {0x00D3, 0xD3}, {0x00D4, 0xD4}, {0x00D6, 0xD6}, {0x00D7, 0xD7}
        , {0x00D9, 0xD9}, {0x00DA, 0xDA}, {0x00DB, 0xDB}, {0x00DC, 0xDC}
        , {0x00DF, 0xDF}, {0x00E0, 0xE0}, {0x00E1, 0xE1}, {0x00E2, 0xE2}
        , {0x00E4, 0xE4}, {0x00E7, 0xE7}, {0x00E8, 0xE8}, {0x00E9, 0xE9}
        , {0x00EA, 0xEA}, {0x00EB, 0xEB}, {0x00EC, 0xEC}, {0x00ED, 0xED}
        , {0x00EE, 0xEE}, {0x00EF, 0xEF}, {0x00F1, 0xF1}, {0x00F2, 0xF2}
        , {0x00F3, 0xF3}, {0x00F4, 0xF4}, {0x00F6, 0xF6}, {0x00F7, 0xF7}
        , {0x00F9, 0xF9}, {0x00FA, 0xFA}, {0x00FB, 0xFB}, {0x00FC, 0xFC}
        , {0x0108, 0xC6}, {0x0109, 0xE6}, {0x010A, 0xC5}, {0x010B, 0xE5}
        , {0x011C, 0xD8}, {0x011D, 0xF8}, {0x011E, 0xAB}, {0x011F, 0xBB}
        , {0x0120, 0xD5}, {0x0121, 0xF5}, {0x0124, 0xA6}, {0x0125, 0xB6}
        , {0x0126, 0xA1}, {0x0127, 0xB1}, {0x0130, 0xA9}, {0x0131, 0xB9}
        , {0x0134, 0xAC}, {0x0135, 0xBC}, {0x015C, 0xDE}, {0x015D, 0xFE}
        , {0x015E, 0xAA}, {0x015F, 0xBA}, {0x016C, 0xDD}, {0x016D, 0xFD}
        , {0x017B, 0xAF}, {0x017C, 0xBF}, {0x02D8, 0xA2}, {0x02D9, 0xFF} };

    const ch32_to_char* enc_map_end = enc_map + detail::array_size(enc_map);
    auto it = std::lower_bound( enc_map
                              , enc_map_end
                              , ch32_to_char{ch, 0}
                              , cmp_ch32_to_char{} );
    return it != enc_map_end && it->key == ch ? it->value : 0x100;
}

BOOST_STRINGIFY_INLINE char32_t impl_iso8859_3::decode(std::uint8_t ch)
{
    if (ch < 0xA1)
    {
        return ch;
    }
    else
    {
        constexpr short undef = -1;
        static const short ext[] =
            { /* A0*/ 0x0126, 0x02D8, 0x00A3, 0x00A4,  undef, 0x0124, 0x00A7
            , 0x00A8, 0x0130, 0x015E, 0x011E, 0x0134, 0x00AD,  undef, 0x017B
            , 0x00B0, 0x0127, 0x00B2, 0x00B3, 0x00B4, 0x00B5, 0x0125, 0x00B7
            , 0x00B8, 0x0131, 0x015F, 0x011F, 0x0135, 0x00BD,  undef, 0x017C
            , 0x00C0, 0x00C1, 0x00C2,  undef, 0x00C4, 0x010A, 0x0108, 0x00C7
            , 0x00C8, 0x00C9, 0x00CA, 0x00CB, 0x00CC, 0x00CD, 0x00CE, 0x00CF
            ,  undef, 0x00D1, 0x00D2, 0x00D3, 0x00D4, 0x0120, 0x00D6, 0x00D7
            , 0x011C, 0x00D9, 0x00DA, 0x00DB, 0x00DC, 0x016C, 0x015C, 0x00DF
            , 0x00E0, 0x00E1, 0x00E2,  undef, 0x00E4, 0x010B, 0x0109, 0x00E7
            , 0x00E8, 0x00E9, 0x00EA, 0x00EB, 0x00EC, 0x00ED, 0x00EE, 0x00EF
            ,  undef, 0x00F1, 0x00F2, 0x00F3, 0x00F4, 0x0121, 0x00F6, 0x00F7
            , 0x011D, 0x00F9, 0x00FA, 0x00FB, 0x00FC, 0x016D, 0x015D, 0x02D9 };

        return (int) ext[ch - 0xA1];
    }
}


class impl_iso8859_15
{
public:

    static bool is_valid(std::uint8_t)
    {
        return true;
    }

    static char32_t decode(std::uint8_t ch)
    {
        static const unsigned short ext[] = {
            /*                           */ 0x20AC, 0x00A5, 0x0160, 0x00A7,
            0x0161, 0x00A9, 0x00AA, 0x00AB, 0x00AC, 0x00AD, 0x00AE, 0x00AF,
            0x00B0, 0x00B1, 0x00B2, 0x00B3, 0x017D, 0x00B5, 0x00B6, 0x00B7,
            0x017E, 0x00B9, 0x00BA, 0x00BB, 0x0152, 0x0153, 0x0178
        };

        if (ch <= 0xA3 || 0xBF <= ch)
            return ch;

        return ext[ch - 0xA4];
    }

    static unsigned encode(char32_t ch)
    {
        return (ch < 0xA0 || (0xBE < ch && ch < 0x100)) ? ch : encode_ext(ch);
    }

private:

    static unsigned encode_ext(char32_t ch);
};

BOOST_STRINGIFY_INLINE unsigned impl_iso8859_15::encode_ext(char32_t ch)
{
    switch(ch)
    {
        case 0x20AC: return 0xA4;
        case 0x0160: return 0xA6;
        case 0x0161: return 0xA8;
        case 0x017D: return 0xB4;
        case 0x017E: return 0xB8;
        case 0x0152: return 0xBC;
        case 0x0153: return 0xBD;
        case 0x0178: return 0xBE;
        case 0xA4:
        case 0xA6:
        case 0xA8:
        case 0xB4:
        case 0xB8:
        case 0xBC:
        case 0xBD:
        case 0xBE:
            return 0x100;
    }
    return ch;
}

class impl_windows_1252
{
public:
    // https://www.unicode.org/Public/MAPPINGS/VENDORS/MICSFT/WindowsBestFit/bestfit1252.txt

    constexpr static unsigned short decode_fail = 0xFFFF;

    static bool is_valid(std::uint8_t)
    {
        return true;
    }

    static char32_t decode(std::uint8_t ch)
    {
        if (ch < 0x80 || 0x9F < ch)
        {
            return ch;
        }
        else
        {
            static const unsigned short ext[] = {
                0x20AC, 0x0081, 0x201A, 0x0192, 0x201E, 0x2026, 0x2020, 0x2021,
                0x02C6, 0x2030, 0x0160, 0x2039, 0x0152, 0x008D, 0x017D, 0x008F,
                0X0090, 0x2018, 0x2019, 0x201C, 0x201D, 0x2022, 0x2013, 0x2014,
                0x02DC, 0x2122, 0x0161, 0x203A, 0x0153, 0x009D, 0x017E, 0x0178,
            };
            return ext[ch - 0x80];
        }
    }

    static unsigned encode(char32_t ch)
    {
        return (ch < 0x80 || (0x9F < ch && ch < 0x100)) ? ch : encode_ext(ch);
    }

private:

    static unsigned encode_ext(char32_t ch);
};

BOOST_STRINGIFY_INLINE unsigned impl_windows_1252::encode_ext(char32_t ch)
{
    switch(ch)
    {
        case 0x81: return 0x81;
        case 0x8D: return 0x8D;
        case 0x8F: return 0x8F;
        case 0x90: return 0x90;
        case 0x9D: return 0x9D;
        case 0x20AC: return 0x80;
        case 0x201A: return 0x82;
        case 0x0192: return 0x83;
        case 0x201E: return 0x84;
        case 0x2026: return 0x85;
        case 0x2020: return 0x86;
        case 0x2021: return 0x87;
        case 0x02C6: return 0x88;
        case 0x2030: return 0x89;
        case 0x0160: return 0x8A;
        case 0x2039: return 0x8B;
        case 0x0152: return 0x8C;
        case 0x017D: return 0x8E;
        case 0x2018: return 0x91;
        case 0x2019: return 0x92;
        case 0x201C: return 0x93;
        case 0x201D: return 0x94;
        case 0x2022: return 0x95;
        case 0x2013: return 0x96;
        case 0x2014: return 0x97;
        case 0x02DC: return 0x98;
        case 0x2122: return 0x99;
        case 0x0161: return 0x9A;
        case 0x203A: return 0x9B;
        case 0x0153: return 0x9C;
        case 0x017E: return 0x9E;
        case 0x0178: return 0x9F;
    }
    return 0x100;
}

BOOST_STRINGIFY_INLINE
const stringify::v0::detail::encoding_impl<std::uint8_t>& windows_1252_impl()
{
    using impl = detail::single_byte_encoding<detail::impl_windows_1252>;
    static const stringify::v0::detail::encoding_impl<std::uint8_t> info =
         { { impl::from_utf32, detail::same_size<char32_t> }
         , { impl::to_utf32,   detail::same_size<std::uint8_t> }
         , { impl::sanitize,   detail::same_size<std::uint8_t> }
         , impl::validate
         , impl::encode_char
         , impl::encode_fill
         , impl::codepoints_count
         , impl::write_replacement_char
         , impl::decode_single_char
         , nullptr, nullptr, nullptr, nullptr, nullptr, nullptr
         , "windows-1252"
         , stringify::v0::encoding_id::eid_windows_1252
         , 1, 0x0, 0x7F };
    return info;
}

BOOST_STRINGIFY_INLINE
const stringify::v0::detail::encoding_impl<std::uint8_t>& iso_8859_1_impl()
{
    using impl = detail::single_byte_encoding<detail::impl_iso8859_1>;
    static const stringify::v0::detail::encoding_impl<std::uint8_t> info =
         { { impl::from_utf32, detail::same_size<char32_t> }
         , { impl::to_utf32,   detail::same_size<std::uint8_t> }
         , { impl::sanitize,   detail::same_size<std::uint8_t> }
         , impl::validate
         , impl::encode_char
         , impl::encode_fill
         , impl::codepoints_count
         , impl::write_replacement_char
         , impl::decode_single_char
         , nullptr, nullptr, nullptr, nullptr, nullptr, nullptr
         , "ISO-8859-1"
         , stringify::v0::encoding_id::eid_iso_8859_1
         , 1, 0x0, 0xFF };
    return info;
}

BOOST_STRINGIFY_INLINE
const stringify::v0::detail::encoding_impl<std::uint8_t>& iso_8859_3_impl()
{
    using impl = detail::single_byte_encoding<detail::impl_iso8859_3>;
    static const stringify::v0::detail::encoding_impl<std::uint8_t> info =
         { { impl::from_utf32, detail::same_size<char32_t> }
         , { impl::to_utf32,   detail::same_size<std::uint8_t> }
         , { impl::sanitize,   detail::same_size<std::uint8_t> }
         , impl::validate
         , impl::encode_char
         , impl::encode_fill
         , impl::codepoints_count
         , impl::write_replacement_char
         , impl::decode_single_char
         , nullptr, nullptr, nullptr, nullptr, nullptr, nullptr
         , "ISO-8859-3"
         , stringify::v0::encoding_id::eid_iso_8859_3
         , 1, 0x0, 0xA0 };
    return info;
}



BOOST_STRINGIFY_INLINE
const stringify::v0::detail::encoding_impl<std::uint8_t>& iso_8859_15_impl()
{
    using impl = detail::single_byte_encoding<detail::impl_iso8859_15>;
    static const stringify::v0::detail::encoding_impl<std::uint8_t> info =
         { { impl::from_utf32, detail::same_size<char32_t> }
         , { impl::to_utf32, detail::same_size<std::uint8_t> }
         , { impl::sanitize, detail::same_size<std::uint8_t> }
         , impl::validate
         , impl::encode_char
         , impl::encode_fill
         , impl::codepoints_count
         , impl::write_replacement_char
         , impl::decode_single_char
         , nullptr, nullptr, nullptr, nullptr, nullptr, nullptr
         , "ISO-8859-15"
         , stringify::v0::encoding_id::eid_iso_8859_15
         , 1, 0x0, 0xA3 };
    return info;
}

} // namespace detail
BOOST_STRINGIFY_V0_NAMESPACE_END

#endif  // BOOST_STRINGIFY_V0_DETAIL_SINGLE_BYTE_ENCODINGS_HPP
