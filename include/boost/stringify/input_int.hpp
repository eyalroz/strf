#ifndef BOOST_STRINGIFY_INPUT_INT_HPP_INCLUDED
#define BOOST_STRINGIFY_INPUT_INT_HPP_INCLUDED

//  Distributed under the Boost Software License, Version 1.0.
//  (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)

#include <boost/stringify/custom_alignment.hpp>
#include <boost/stringify/custom_base_indication.hpp>
#include <boost/stringify/custom_fill.hpp>
#include <boost/stringify/custom_intbase.hpp>
#include <boost/stringify/custom_case.hpp>
#include <boost/stringify/custom_showbase.hpp>
#include <boost/stringify/custom_showpos.hpp>
#include <boost/stringify/custom_width.hpp>
#include <boost/stringify/ftuple.hpp>
#include <boost/stringify/stringifier.hpp>
#include <boost/stringify/detail/characters_catalog.hpp>
#include <boost/stringify/detail/char_flags.hpp>
#include <boost/stringify/detail/number_of_digits.hpp>
#include <cstdint>

namespace boost {
namespace stringify {
namespace detail {


constexpr unsigned global_buff_size = 6 * sizeof(std::uintmax_t);
inline char32_t* global_buff()
{
    static thread_local char32_t arr[global_buff_size];
    return arr;
}


struct int_arg_format
{
    typedef boost::stringify::width_t width_t;
    typedef boost::stringify::detail::char_flags
        <'+', '-', '<', '>', '=', 'o', 'd', 'x', 'X', 'c', 'C', '#', '$'>
        char_flags_type;

    constexpr int_arg_format(const char* flags, width_t w = -1
        )
        : m_flags(flags)
        , m_width(w)
    {
    }

    constexpr int_arg_format(boost::stringify::width_t w)
        : m_width(w)
    {
    }

    constexpr int_arg_format()
    {
    }

    constexpr int_arg_format(const int_arg_format&) = default;

    int_arg_format& operator=(const int_arg_format&) = default;

    template <typename IntputType, typename FTuple>
    bool get_showpos(const FTuple& fmt) const noexcept
    {
        if (m_flags.has_char('-'))
        {
            return false;
        }
        else if (m_flags.has_char('+'))
        {
            return true;
        }
        else
        {
            return boost::stringify::get_showpos<IntputType>(fmt);
        }
    }

    char_flags_type m_flags;
    boost::stringify::width_t m_width = -1;
};


template
    < typename intT
    , typename unsigned_intT = typename std::make_unsigned<intT>::type
    >
typename std::enable_if<std::is_signed<intT>::value, unsigned_intT>::type
unsigned_abs(intT value)
{
    return ( value > 0
           ? static_cast<unsigned_intT>(value)
           : 1 + static_cast<unsigned_intT>(-(value + 1)));
}


template<typename intT>
typename std::enable_if<std::is_unsigned<intT>::value, intT>::type
unsigned_abs(intT value)
{
    return value;
}


template <typename intT, typename CharT, typename Output, typename Formatting>
struct int_stringifier
    : public boost::stringify::stringifier<CharT, Output, Formatting>
{
    using base_class = boost::stringify::stringifier<CharT, Output, Formatting>;
    using unsigned_intT = typename std::make_unsigned<intT>::type;
    using width_t = boost::stringify::width_t;
    using base_class::noexcept_output;
    using base_class::random_access_output;
    using width_tag = boost::stringify::width_tag;
    using chars_catalog = boost::stringify::detail::characters_catalog;
    static constexpr bool is_signed = std::is_signed<intT>::value;

public:

    using input_type  = intT ;
    using har_type    = CharT ;
    using output_type = Output;
    using ftuple_type = Formatting;
    using arg_format_type = boost::stringify::detail::int_arg_format;


    int_stringifier(const Formatting& fmt, intT value) noexcept
        : m_fmt(fmt)
        , m_value(value)
        , m_width(get_facet<width_tag>().width())
        , m_alignment(boost::stringify::get_alignment<input_type>(fmt))
        , m_base(boost::stringify::get_intbase<input_type>(fmt))
        , m_showpos
            ( is_signed
           && value >= 0
           && boost::stringify::get_showpos<input_type>(fmt)
            )
        , m_showbase(boost::stringify::get_showbase<input_type>(fmt))
        , m_uppercase(boost::stringify::get_uppercase<input_type>(fmt))

    {
    }


    int_stringifier(const Formatting& fmt, intT value, arg_format_type argf) noexcept
        : m_fmt(fmt)
        , m_value(value)
        , m_width(argf.m_width > -1 ? argf.m_width : get_facet<width_tag>().width())
        , m_alignment(boost::stringify::get_alignment<input_type>(fmt, argf.m_flags))
        , m_base(boost::stringify::get_intbase<input_type>(fmt, argf.m_flags))
        , m_showpos
            ( is_signed
           && value >= 0
           && boost::stringify::get_showpos<input_type>(fmt, argf.m_flags)
            )
        , m_showbase(boost::stringify::get_showbase<input_type>(fmt, argf.m_flags))
        , m_uppercase(boost::stringify::get_uppercase<input_type>(fmt, argf.m_flags))
    {
    }


    virtual std::size_t length() const noexcept override
    {
        return length_body() + length_fill();
    }


    virtual void write(Output& out) const noexcept(noexcept_output) override
    {
        width_t fill_width = 0;
        if (m_width > 0)
        {
            fill_width = m_width - width_body();
        }

        if (fill_width > 0)
        {
            write(out, fill_width);
        }
        else
        {
            write_without_fill(out);
        }
    }

private:

    const Formatting& m_fmt;
    const intT m_value;
    const width_t m_width;
    const boost::stringify::alignment m_alignment;
    const unsigned short m_base;
    const bool m_showpos;
    const bool m_showbase;
    const bool m_uppercase;


    template <typename fmt_tag>
    decltype(auto) get_facet() const noexcept
    {
        return m_fmt.template get<fmt_tag, input_type>();
    }


    constexpr unsigned_intT unsigned_value() const noexcept
    {
        if(m_base == 10)
        {
            return boost::stringify::detail::unsigned_abs(m_value);
        }
        return static_cast<unsigned_intT>(m_value);
    }


    std::size_t length_fill() const
    {
        width_t fill_width = m_width > 0 ? m_width - width_body() : 0;
        if (fill_width > 0)
        {
            return boost::stringify::fill_length<CharT, input_type>
                (fill_width, m_fmt);
        }
        return 0;
    }


    std::size_t length_body() const
    {
        switch(m_base)
        {
            case 16:
                return length_digits<16>() + (m_showbase ? 2 : 0);
            case  8:
                return length_digits<8>() + (m_showbase ? 1 : 0);
        }
        BOOST_ASSERT(m_base == 10);
        return length_digits<10>() + (m_value < 0 || m_showpos ? 1 : 0);
    }


    template <unsigned Base>
    std::size_t length_digits() const noexcept
    {
        return boost::stringify::detail::number_of_digits<Base>(unsigned_value());
    }


    // template <unsigned Base>
    // std::size_t length_separators(unsigned num_digits) const noexcept
    // {
    //     auto separators_count =
    //         boost::stringify::numgrouping_count<input_type, Base>
    //         (m_fmt, num_digits);

    //     if (separators_count > 0)
    //     {
    //         char32_t separator
    //             = boost::stringify::thousands_sep<input_type, Base>(m_fmt);
    //         return separators_count
    //             * boost::stringify::get_char32_length<CharT, input_type>
    //               (m_fmt, separator);
    //     }
    //     return 0;
    // }


    void write(Output& out, width_t fill_width) const
    {
        switch (m_alignment)
        {
            case boost::stringify::alignment::left:
                write_sign(out);
                write_base(out);
                write_digits(out);
                write_fill(out, fill_width);
                break;

            case boost::stringify::alignment::right:
                write_fill(out, fill_width);
                write_sign(out);
                write_base(out);
                write_digits(out);
                break;

            default:
                BOOST_ASSERT(m_alignment == boost::stringify::alignment::internal);
                write_sign(out);
                write_base(out);
                write_fill(out, fill_width);
                write_digits(out);
        }
    }


    void write_fill(Output& out, width_t fill_width) const
    {
        boost::stringify::write_fill<CharT, input_type>
                        (fill_width, out, m_fmt);
    }


    void write_sign(Output& out) const noexcept(noexcept_output)
    {
        if (std::is_signed<intT>::value && m_base == 10)
        {
            if (m_value < 0)
            {
                out.put(chars_catalog::minus<CharT>);
            }
            else if(m_showpos)
            {
                out.put(chars_catalog::plus<CharT>);
            }
        }
    }


    void write_without_fill(Output& out) const
    {
        switch(m_base)
        {
            case 16:
                write_base(out);
                write_digits_t<16>(out);
                break;

            case  8:
                write_base(out);
                write_digits_t<8>(out);
                break;

            default:
                BOOST_ASSERT(m_base == 10);
                write_sign(out);
                write_digits_t<10>(out);
        }
    }


    void write_base(Output& out) const
    {
        if(m_showbase)
        {
            if (m_base == 16)
            {
                out.put(chars_catalog::zero<CharT>);
                if (m_uppercase)
                {
                    out.put(chars_catalog::X<CharT>);
                }
                else
                {
                    out.put(chars_catalog::x<CharT>);
                }
            }
            else if(m_base == 8)
            {
                out.put(chars_catalog::zero<CharT>);
            }
        }
    }


    void write_digits(Output& out) const
    {
        switch (m_base)
        {
            case 10 : write_digits_t<10>(out); break;
            case 16 : write_digits_t<16>(out); break;
            default:
                BOOST_ASSERT(m_base == 8);
                write_digits_t<8>(out);
        }
    }


    template <unsigned Base>
    void write_digits_t(Output& out) const
    {
        auto end = buff_end();
        auto begin = write_digits_backwards<Base>(unsigned_value(), end);
        out.put(begin, (end - begin));
    }


    CharT* buff_end() const noexcept
    {
        return reinterpret_cast<CharT*>
            (& boost::stringify::detail::global_buff()
             [boost::stringify::detail::global_buff_size]
            );
    }


    template <unsigned Base, typename OutputIterator>
    OutputIterator*
    write_digits_backwards(unsigned_intT value, OutputIterator* it) const
    {
        while(value >= Base)
        {
            *--it = character_of_digit(value % Base);
            value /= Base;
        }
        *--it = character_of_digit(value);
        return it;
    }


    CharT character_of_digit(unsigned int digit) const noexcept
    {
        if (digit < 10)
        {
            return chars_catalog::zero<CharT> + digit;
        }
        else if (m_uppercase)
        {
            return chars_catalog::A<CharT> + digit - 10;
        }
        return chars_catalog::a<CharT> + digit - 10;
    }


    width_t width_body() const noexcept
    {
        width_t bw = 0;
        auto uv = unsigned_value();
        if(m_base == 10)
        {
            if(m_value < 0 || m_showpos)
            {
                ++bw;
            }
            bw += boost::stringify::detail::number_of_digits<10>(uv);
        }
        else if(m_base == 16)
        {
            if (m_showbase)
            {
                bw += 2;
            }
            bw += boost::stringify::detail::number_of_digits<16>(uv);
        }
        else
        {
            BOOST_ASSERT(m_base == 8);
            if (m_showbase)
            {
                ++bw;
            }
            bw += boost::stringify::detail::number_of_digits<8>(uv);
        }
        return bw;
    }

};


template <typename IntT>
struct int_input_traits
{
    template <typename CharT, typename Output, typename Formatting>
    using stringifier =
        boost::stringify::detail::int_stringifier<IntT, CharT, Output, Formatting>;
};

} // namespace detail

boost::stringify::detail::int_input_traits<short>
boost_stringify_input_traits_of(short);

boost::stringify::detail::int_input_traits<int>
boost_stringify_input_traits_of(int);

boost::stringify::detail::int_input_traits<long>
boost_stringify_input_traits_of(long);

boost::stringify::detail::int_input_traits<long long>
boost_stringify_input_traits_of(long long);

boost::stringify::detail::int_input_traits<unsigned short>
boost_stringify_input_traits_of(unsigned short);

boost::stringify::detail::int_input_traits<unsigned>
boost_stringify_input_traits_of(unsigned);

boost::stringify::detail::int_input_traits<unsigned long>
boost_stringify_input_traits_of(unsigned long);

boost::stringify::detail::int_input_traits<unsigned long long>
boost_stringify_input_traits_of(unsigned long long);

}//namespace stringify
}//namespace boost


#endif
