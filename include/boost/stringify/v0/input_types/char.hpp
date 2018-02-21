#ifndef BOOST_STRINGIFY_V0_INPUT_TYPES_CHAR_HPP_INCLUDED
#define BOOST_STRINGIFY_V0_INPUT_TYPES_CHAR_HPP_INCLUDED

//  Distributed under the Boost Software License, Version 1.0.
//  (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)

#include <boost/stringify/v0/input_types/char32.hpp>

BOOST_STRINGIFY_V0_NAMESPACE_BEGIN
namespace detail {

template <typename CharT>
class char_formatter: public formatter<CharT>
{
    using input_type = CharT;
    using writer_type = stringify::v0::output_writer<CharT>;

public:

    template <typename FTuple>
    char_formatter
        ( const FTuple& ft
        , const stringify::v0::char_with_format<CharT>& input
        ) noexcept
        : char_formatter(input, get_encoder(ft), get_width_calculator(ft))
    {
    }

    char_formatter
        ( const stringify::v0::char_with_format<CharT>& input
        , const stringify::v0::encoder<CharT>& encoder
        , const stringify::v0::width_calculator& wcalc
        ) noexcept;

    virtual ~char_formatter();

    std::size_t length() const override;

    void write(writer_type& out) const override;

    int remaining_width(int w) const override;

private:

    stringify::v0::char_with_format<CharT> m_fmt;
    const stringify::v0::encoder<CharT>& m_encoder;
    int m_fillcount = 0;

    template <typename FTuple>
    static const auto& get_encoder(const FTuple& ft)
    {
        using category = stringify::v0::encoder_category<CharT>;
        return ft.template get_facet<category, input_type>();
    }

    template <typename FTuple>
    static const auto& get_width_calculator(const FTuple& ft)
    {
        using category = stringify::v0::width_calculator_category;
        return ft.template get_facet<category, input_type>();
    }

    void determinate_fill_and_width(const stringify::v0::width_calculator& wcalc)
    {
        int content_width = 0;
        if(m_fmt.width() < 0)
        {
            m_fmt.width(0);
        }
        if (m_fmt.count() > 0 )
        {
            char32_t ch32 = m_fmt.value(); // todo: use convertion_to_utf32 facet ?
            content_width = m_fmt.count() * wcalc.width_of(ch32);
        }
        if (content_width >= m_fmt.width())
        {
            m_fillcount = 0;
            m_fmt.width(content_width);
        }
        else
        {
            m_fillcount = m_fmt.width() - content_width;
        }
    }
};


template <typename CharT>
char_formatter<CharT>::char_formatter
    ( const stringify::v0::char_with_format<CharT>& input
    , const stringify::v0::encoder<CharT>& encoder
    , const stringify::v0::width_calculator& wcalc
    ) noexcept
    : m_fmt(input)
    , m_encoder(encoder)
{
    determinate_fill_and_width(wcalc);
}

template <typename CharT>
char_formatter<CharT>::~char_formatter()
{
}


template <typename CharT>
std::size_t char_formatter<CharT>::length() const
{
    std::size_t len = m_fmt.count();
    if (m_fillcount > 0)
    {
        len += m_fillcount * m_encoder.length(m_fmt.fill());
    }
    return len;
}


template <typename CharT>
void char_formatter<CharT>::write(writer_type& out) const
{
    if (m_fillcount == 0)
    {
        out.repeat(m_fmt.count(), m_fmt.value());
    }
    else
    {
        switch(m_fmt.alignment())
        {
            case stringify::v0::alignment::left:
            {
                out.repeat(m_fmt.count(), m_fmt.value());
                m_encoder.encode(out, m_fillcount, m_fmt.fill());
                break;
            }
            case stringify::v0::alignment::center:
            {
                auto halfcount = m_fillcount / 2;
                m_encoder.encode(out, halfcount, m_fmt.fill());
                m_encoder.encode(out, m_fmt.count(), m_fmt.value());
                m_encoder.encode(out, m_fillcount - halfcount, m_fmt.fill());
                break;
            }
            default:
            {
                m_encoder.encode(out, m_fillcount, m_fmt.fill());
                out.repeat(m_fmt.count(), m_fmt.value());
            }
        }
    }
}


template <typename CharT>
int char_formatter<CharT>::remaining_width(int w) const
{
    if (w > m_fmt.width())
    {
        return w - m_fmt.width();
    }
    return 0;
}



#if defined(BOOST_STRINGIFY_NOT_HEADER_ONLY)

BOOST_STRINGIFY_EXPLICIT_TEMPLATE class char_formatter<char>;
BOOST_STRINGIFY_EXPLICIT_TEMPLATE class char_formatter<char16_t>;
BOOST_STRINGIFY_EXPLICIT_TEMPLATE class char_formatter<char32_t>;
BOOST_STRINGIFY_EXPLICIT_TEMPLATE class char_formatter<wchar_t>;

#endif // defined(BOOST_STRINGIFY_NOT_HEADER_ONLY)


template <typename CharIn>
struct char_input_traits
{

private:

    template <typename CharOut>
    struct checker
    {
        static_assert(sizeof(CharIn) == sizeof(CharOut), "");

        using formatter
        = stringify::v0::detail::char_formatter<CharOut>;
    };

public:

    template <typename CharOut, typename>
    using formatter = typename checker<CharOut>::formatter;

    constexpr static auto fmt(CharIn value)
        -> stringify::v0::char_with_format<CharIn>
    {
        return {value};
    }

};

} //namepace detail


stringify::v0::detail::char_input_traits<char>
boost_stringify_input_traits_of(char);

stringify::v0::detail::char_input_traits<char16_t>
boost_stringify_input_traits_of(char16_t);

stringify::v0::detail::char_input_traits<wchar_t>
boost_stringify_input_traits_of(wchar_t);

template <typename CharT>
stringify::v0::detail::char_input_traits<CharT>
boost_stringify_input_traits_of(const stringify::v0::char_with_format<CharT>&);


BOOST_STRINGIFY_V0_NAMESPACE_END

#endif // BOOST_STRINGIFY_V0_INPUT_TYPES_CHAR_HPP_INCLUDED



