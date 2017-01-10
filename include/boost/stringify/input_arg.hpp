#ifndef BOOST_STRINGIFY_INPUT_ARG_HPP
#define BOOST_STRINGIFY_INPUT_ARG_HPP

//  Distributed under the Boost Software License, Version 1.0.
//  (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)

#include <boost/stringify/input_base.hpp>

namespace boost {
namespace stringify {

template <class T>
struct input_traits;

// template <class T>
// boost::stringify::input_traits<T>
// boost_stringify_input_traits_of(...);


template <typename CharT, typename Output, typename Formatting>
class input_arg
{
public:
 
    template <class T>
    using stringificator = typename decltype
        (boost_stringify_input_traits_of(std::declval<const T>()))
        :: template stringificator<CharT, Output, Formatting>;

    
    template <typename T>
    input_arg
        ( const T& value
        , stringificator<T> && wt = stringificator<T>() // will be stringificator<T>(value)
                                                        // after P0145R2 is supported
        )
        noexcept
        : writer(wt)
    {
        wt.set(value); // will be removed after compilers add support to P0145R2
    }
   
    template <typename T>
    input_arg
        ( const T& arg
        , const typename stringificator<T>::local_formatting& arg_format
        , stringificator<T> && wt = stringificator<T>() // will be stringificator<T>(arg, arg_format)
        ) noexcept
        : writer(wt)
    {
        wt.set(arg, arg_format);  // will be removed after compilers add support to P0145R2
    }

    const boost::stringify::input_base<CharT, Output, Formatting>& writer;
};


} // namespace stringify
} // namespace boost

#endif  /* BOOST_STRINGIFY_DETAIL_STR_WRITE_REF_HPP */

