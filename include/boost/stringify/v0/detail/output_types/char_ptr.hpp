#ifndef BOOST_STRINGIFY_V0_DETAIL_OUTPUT_TYPES_CHAR_PTR_HPP
#define BOOST_STRINGIFY_V0_DETAIL_OUTPUT_TYPES_CHAR_PTR_HPP

//  Distributed under the Boost Software License, Version 1.0.
//  (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)

#include <string>
#include <boost/stringify/v0/dispatcher.hpp>
#include <boost/stringify/v0/outbuf.hpp>

BOOST_STRINGIFY_V0_NAMESPACE_BEGIN

namespace detail {

template <typename CharT>
class basic_cstr_writer_factory
{
public:

    using char_type = CharT;
    using finish_type = typename basic_cstr_writer<CharT>::result;
    
    constexpr basic_cstr_writer_factory(CharT* dest, CharT* dest_end) noexcept 
        : _dest(dest)
        , _dest_end(dest_end)
    {
        BOOST_ASSERT(dest < dest_end);
    }

    constexpr basic_cstr_writer_factory(const basic_cstr_writer_factory&) = default;

    template <typename ... Printers>
    finish_type write(const Printers& ... printers) const
    {
        basic_cstr_writer<CharT> ob(_dest, _dest_end);
        stringify::v0::detail::write_args(ob, printers...);;
        return ob.finish();
    }
    
private:

    CharT* _dest;
    CharT* _dest_end;
};

}

#if defined(__cpp_char8_t)

template<std::size_t N>
inline auto write(char8_t (&dest)[N])
{
    return stringify::v0::dispatcher_no_reserve
        < stringify::v0::detail::basic_cstr_writer_factory<char8_t> >
        (dest, dest + N);
}

inline auto write(char8_t* dest, char8_t* end)
{
    return stringify::v0::dispatcher_no_reserve
        < stringify::v0::detail::basic_cstr_writer_factory<char8_t> >
}

inline auto write(char8_t* dest, std::size_t count)
{
    return stringify::v0::dispatcher_no_reserve
        < stringify::v0::detail::basic_cstr_writer_factory<char8_t> >
        (dest, dest + count);
}

#endif

template<std::size_t N>
inline auto write(char (&dest)[N])
{
    return stringify::v0::dispatcher_no_reserve
        < stringify::v0::detail::basic_cstr_writer_factory<char> >
        (dest, dest + N);
}

inline auto write(char* dest, char* end)
{
    return stringify::v0::dispatcher_no_reserve
        < stringify::v0::detail::basic_cstr_writer_factory<char> >
        (dest, end);
}

inline auto write(char* dest, std::size_t count)
{
    return stringify::v0::dispatcher_no_reserve
        < stringify::v0::detail::basic_cstr_writer_factory<char> >
        (dest, dest + count);
}

template<std::size_t N>
inline auto write(char16_t (&dest)[N])
{
    return stringify::v0::dispatcher_no_reserve
        < stringify::v0::detail::basic_cstr_writer_factory<char16_t> >  
        (dest, dest + N);
}

inline auto write(char16_t* dest, char16_t* end)
{
    return stringify::v0::dispatcher_no_reserve
        < stringify::v0::detail::basic_cstr_writer_factory<char16_t> >
        (dest, end);
}

inline auto write(char16_t* dest, std::size_t count)
{
    return stringify::v0::dispatcher_no_reserve
        < stringify::v0::detail::basic_cstr_writer_factory<char16_t> >  
        (dest, dest + count);
}

template<std::size_t N>
inline auto write(char32_t (&dest)[N])
{
    return stringify::v0::dispatcher_no_reserve
        < stringify::v0::detail::basic_cstr_writer_factory<char32_t> >
        (dest, dest + N);
}

inline auto write(char32_t* dest, char32_t* end)
{
    return stringify::v0::dispatcher_no_reserve
        < stringify::v0::detail::basic_cstr_writer_factory<char32_t> >  
        (dest, end);
}

inline auto write(char32_t* dest, std::size_t count)
{
    return stringify::v0::dispatcher_no_reserve
        < stringify::v0::detail::basic_cstr_writer_factory<char32_t> >
        (dest, dest + count);
}

template<std::size_t N>
inline auto write(wchar_t (&dest)[N])
{
    return stringify::v0::dispatcher_no_reserve
        < stringify::v0::detail::basic_cstr_writer_factory<wchar_t> >
        (dest, dest + N);
}

inline auto write(wchar_t* dest, wchar_t* end)
{
    return stringify::v0::dispatcher_no_reserve
        < stringify::v0::detail::basic_cstr_writer_factory<wchar_t> >
        (dest, end);
}

inline auto write(wchar_t* dest, std::size_t count)
{
    return stringify::v0::dispatcher_no_reserve
        < stringify::v0::detail::basic_cstr_writer_factory<wchar_t> >
        (dest, dest + count);
}

BOOST_STRINGIFY_V0_NAMESPACE_END

#endif  /* BOOST_STRINGIFY_V0_DETAIL_OUTPUT_TYPES_CHAR_PTR_HPP */

