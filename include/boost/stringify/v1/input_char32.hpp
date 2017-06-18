#ifndef BOOST_STRINGIFY_V1_INPUT_CHAR32_HPP_INCLUDED
#define BOOST_STRINGIFY_V1_INPUT_CHAR32_HPP_INCLUDED

//  Distributed under the Boost Software License, Version 1.0.
//  (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)

#include <boost/stringify/v1/custom_char32_conversion.hpp>
#include <type_traits>

namespace boost {
namespace stringify {
inline namespace v1 {
namespace detail {

template <typename CharT, typename Output, typename FTuple>
class char32_stringifier
{

public:

    using input_type = char32_t;
    using char_type = CharT;
    using output_type = Output;
    using ftuple_type = FTuple;
    
    char32_stringifier(const FTuple& fmt, char32_t ch) noexcept
        : m_fmt(fmt)
        , m_char32(ch)
    {
    }

    std::size_t length() const
    {
        return boost::stringify::v1::get_char32_writer<CharT, char32_t>(m_fmt)
            .length(m_char32);
    }
    
    void write(Output& out) const
    {
        return boost::stringify::v1::get_char32_writer<CharT, char32_t>(m_fmt)
            .write(m_char32, out);
    }

    int remaining_width(int w) const
    {
        auto calc = boost::stringify::v1::get_width_calculator<input_type>(m_fmt);
        return w - calc.width_of(m_char32);
    }


private:

    const FTuple& m_fmt;
    char32_t m_char32;

};


struct char32_input_traits
{
    template <typename CharT, typename Output, typename FTuple>
    using stringifier
    = boost::stringify::v1::detail::char32_stringifier
        <CharT, Output, FTuple>;
};

} // namespace detail

boost::stringify::v1::detail::char32_input_traits
boost_stringify_input_traits_of(char32_t);


} // inline namespace v1
} // namespace stringify
} // namespace boost

#endif // BOOST_STRINGIFY_V1_INPUT_CHAR32_HPP_INCLUDED


