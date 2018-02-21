#ifndef BOOST_STRINGIFY_V0_FACETS_WIDTH_CALCULATOR_HPP
#define BOOST_STRINGIFY_V0_FACETS_WIDTH_CALCULATOR_HPP

//  Distributed under the Boost Software License, Version 1.0.
//  (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)

#include <boost/stringify/v0/facets/decoder.hpp>
#include <boost/stringify/v0/ftuple.hpp>
#include <boost/assert.hpp>
#include <string>
#include <limits>
#include <algorithm>

BOOST_STRINGIFY_V0_NAMESPACE_BEGIN

struct width_calculator_category;
class width_calculator;


typedef int (*char_width_calculator)(char32_t);

namespace detail{

class width_decrementer: public stringify::v0::u32output
{
public:

    width_decrementer
        ( int initial_width
        , const stringify::v0::char_width_calculator wc
        )
        : m_wcalc(wc)
        , m_width(initial_width)
    {
    }

    ~width_decrementer()
    {
    }

    bool put(char32_t ch) override;

    void set_error(std::error_code err) override;

    int get_remaining_width() const
    {
        return m_width > 0 ? m_width : 0;
    }

private:

    const stringify::v0::char_width_calculator m_wcalc;
    bool m_err = false;
    int m_width = 0;

};

} // namespaced detail


class width_calculator
{

public:

    typedef stringify::v0::width_calculator_category category;

    explicit width_calculator
    ( const stringify::v0::char_width_calculator ch_wcalc
    )
        : m_ch_wcalc(ch_wcalc)
    {
    }

    width_calculator(const width_calculator& cp)
        : m_ch_wcalc(cp.m_ch_wcalc)
    {
    }

    int width_of(char32_t ch) const;

    int remaining_width
        ( int width
        , const char32_t* begin
        , const char32_t* end
        ) const;

    template <typename CharT>
    int remaining_width
        ( int width
        , const CharT* begin
        , const CharT* end
        , const stringify::v0::decoder<CharT>& conv
        ) const
    {
        if (m_ch_wcalc == nullptr)
        {
            std::size_t str_len = end - begin;
            return str_len > (std::size_t)(width) ? 0 : width - static_cast<int>(str_len);
        }
        //else
        {
            detail::width_decrementer decrementer{width, *m_ch_wcalc};
            conv.decode(decrementer, begin, end);
            return decrementer.get_remaining_width();
        }
    }

private:

    const stringify::v0::char_width_calculator m_ch_wcalc;
};


struct width_calculator_category
{
    static const stringify::v0::width_calculator& get_default()
    {
        static stringify::v0::width_calculator x {nullptr};
        return x;
    }
};

#if defined(BOOST_STRINGIFY_NOT_HEADER_ONLY)

BOOST_STRINGIFY_EXPLICIT_TEMPLATE
int width_calculator::remaining_width<char>
    ( int width
    , const char* begin
    , const char* end
    , const stringify::v0::decoder<char>& conv
    ) const;

BOOST_STRINGIFY_EXPLICIT_TEMPLATE
int width_calculator::remaining_width<char16_t>
    ( int width
    , const char16_t* begin
    , const char16_t* end
    , const stringify::v0::decoder<char16_t>& conv
    ) const;

BOOST_STRINGIFY_EXPLICIT_TEMPLATE
int width_calculator::remaining_width<char32_t>
    ( int width
    , const char32_t* begin
    , const char32_t* end
    , const stringify::v0::decoder<char32_t>& conv
    ) const;

BOOST_STRINGIFY_EXPLICIT_TEMPLATE
int width_calculator::remaining_width<wchar_t>
    ( int width
    , const wchar_t* str
    , const wchar_t* end
    , const stringify::v0::decoder<wchar_t>& conv
    ) const;

#endif // defined(BOOST_STRINGIFY_NOT_HEADER_ONLY)


#if ! defined(BOOST_STRINGIFY_OMIT_IMPL)

namespace detail {

BOOST_STRINGIFY_INLINE bool width_decrementer::put(char32_t ch)
{
    if (m_err)
    {
        return false;
    }
    m_width -= m_wcalc(ch);
    return m_width > 0;
}

BOOST_STRINGIFY_INLINE void width_decrementer::set_error(std::error_code)
{
    m_err = true;
}

} // namespace detail

BOOST_STRINGIFY_INLINE int width_calculator::width_of(char32_t ch) const
{
    if (m_ch_wcalc == nullptr)
    {
        return 1;
    }
    else
    {
        return m_ch_wcalc(ch);
    }
}


BOOST_STRINGIFY_INLINE int width_calculator::remaining_width
    ( int width
    , const char32_t* begin
    , const char32_t* end
    ) const
{
    if (m_ch_wcalc == nullptr)
    {
        std::size_t str_len = end - begin;
        if(str_len > (std::size_t)(width))
        {
            return 0;
        }
        return width - static_cast<int>(str_len);
    }
    else
    {
        for(auto it = begin; it < end; ++it)
        {
            width -= m_ch_wcalc(*it);
        }
        return width > 0 ? width : 0;
    }
}

#endif // ! defined(BOOST_STRINGIFY_OMIT_IMPL)

namespace detail{

inline int char_width_aways_one(char32_t)
{
    return 1;
}

}

inline stringify::v0::width_calculator width_as_length()
{
    return stringify::v0::width_calculator{nullptr};
}

inline stringify::v0::width_calculator width_as_codepoints_count()
{
    return stringify::v0::width_calculator
        { stringify::v0::detail::char_width_aways_one };
}

inline stringify::v0::width_calculator width_as
    (stringify::v0::char_width_calculator func)
{
    return stringify::v0::width_calculator {func};
}


BOOST_STRINGIFY_V0_NAMESPACE_END

#endif  // BOOST_STRINGIFY_V0_FACETS_WIDTH_CALCULATOR_HPP

