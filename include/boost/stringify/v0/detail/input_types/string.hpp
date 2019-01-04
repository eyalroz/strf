#ifndef BOOST_STRINGIFY_V0_DETAIL_INPUT_TYPES_STRING
#define BOOST_STRINGIFY_V0_DETAIL_INPUT_TYPES_STRING

#include <algorithm>
#include <limits>
#include <boost/stringify/v0/detail/string_format.hpp>
#include <boost/stringify/v0/detail/facets/width_calculator.hpp>

BOOST_STRINGIFY_V0_NAMESPACE_BEGIN

namespace detail {

template <typename CharT>
class string_printer: public stringify::v0::printer<CharT>
{
public:

    template <typename FPack>
    string_printer
        ( const FPack& fp
        , const CharT* str
        , std::size_t len ) noexcept
        : _str(str)
        , _len(len)
        , _wcalc(_get_facet<stringify::v0::width_calculator_category>(fp))
        , _encoding(_get_facet<stringify::v0::encoding_category<CharT>>(fp))
        , _epoli(_get_facet<stringify::v0::encoding_policy_category>(fp))
    {
    }

    std::size_t necessary_size() const override;

    stringify::v0::expected_output_buffer<CharT> write
        ( stringify::v0::output_buffer<CharT> buff
        , stringify::buffer_recycler<CharT>& recycler ) const override;

    int remaining_width(int w) const override;

private:

    const CharT* _str;
    const std::size_t _len;
    const stringify::v0::width_calculator _wcalc;
    const stringify::v0::encoding<CharT> _encoding;
    const stringify::v0::encoding_policy  _epoli;

    template <typename Category, typename FPack>
    const auto& _get_facet(const FPack& fp) const
    {
        using input_tag = stringify::v0::string_input_tag<CharT>;
        return fp.template get_facet<Category, input_tag>();
    }
};

template<typename CharT>
std::size_t string_printer<CharT>::necessary_size() const
{
    return _len;
}

template<typename CharT>
stringify::v0::expected_output_buffer<CharT>
string_printer<CharT>::write
    ( stringify::v0::output_buffer<CharT> buff
    , stringify::buffer_recycler<CharT>& recycler ) const
{
    return stringify::v0::detail::write_str(buff, recycler, _str, _len);
}

template<typename CharT>
int string_printer<CharT>::remaining_width(int w) const
{
    return _wcalc.remaining_width(w, _str, _len, _encoding, _epoli);
}

template <typename CharT>
class fmt_string_printer: public stringify::v0::printer<CharT>
{
public:

    template <typename FPack>
    fmt_string_printer
        ( const FPack& fp
        , const stringify::v0::string_with_format<CharT>& input )
        : _fmt(input)
        , _wcalc(_get_facet<stringify::v0::width_calculator_category>(fp))
        , _encoding(_get_facet<stringify::v0::encoding_category<CharT>>(fp))
        , _epoli(_get_facet<stringify::v0::encoding_policy_category>(fp))
    {
        _init();
    }

    ~fmt_string_printer();

    std::size_t necessary_size() const override;

    stringify::v0::expected_output_buffer<CharT> write
        ( stringify::v0::output_buffer<CharT> buff
        , stringify::buffer_recycler<CharT>& recycler ) const override;

    int remaining_width(int w) const override;

private:

    const stringify::v0::string_with_format<CharT> _fmt;
    const stringify::v0::width_calculator _wcalc;
    const stringify::v0::encoding<CharT> _encoding;
    const stringify::v0::encoding_policy  _epoli;
    unsigned _fillcount = 0;

    template <typename Category, typename FPack>
    const auto& _get_facet(const FPack& fp) const
    {
        using input_tag = stringify::v0::string_input_tag<CharT>;
        return fp.template get_facet<Category, input_tag>();
    }

    void _init();

    stringify::v0::expected_output_buffer<CharT> _write_str
        ( stringify::v0::output_buffer<CharT> buff
        , stringify::buffer_recycler<CharT>& recycler ) const;

    stringify::v0::expected_output_buffer<CharT> _write_fill
        ( stringify::v0::output_buffer<CharT> buff
        , stringify::buffer_recycler<CharT>& recycler
        , unsigned count ) const;
};

template<typename CharT>
fmt_string_printer<CharT>::~fmt_string_printer()
{
}

template<typename CharT>
void fmt_string_printer<CharT>::_init()
{
    _fillcount = ( _fmt.width() > 0
                 ? _wcalc.remaining_width
                     ( _fmt.width()
                     , _fmt.value().begin()
                     , _fmt.value().length()
                     , _encoding
                     , _epoli )
                 : 0 );
}

template<typename CharT>
std::size_t fmt_string_printer<CharT>::necessary_size() const
{
    if (_fillcount > 0)
    {
        return _fillcount * _encoding.char_size(_fmt.fill(), _epoli.err_hdl())
            + _fmt.value().length();
    }
    return _fmt.value().length();
}

template<typename CharT>
int fmt_string_printer<CharT>::remaining_width(int w) const
{
    if (_fillcount > 0)
    {
        return w > _fmt.width() ? w - _fmt.width() : 0;
    }
    return _wcalc.remaining_width( w, _fmt.value().begin(), _fmt.value().length()
                                 , _encoding, _epoli );
}

template<typename CharT>
stringify::v0::expected_output_buffer<CharT>
fmt_string_printer<CharT>::write
    ( stringify::v0::output_buffer<CharT> buff
    , stringify::buffer_recycler<CharT>& recycler ) const
{
    if (_fillcount > 0)
    {
        switch (_fmt.alignment())
        {
            case stringify::v0::alignment::left:
            {
                auto x = _write_str(buff, recycler);
                return x ? _write_fill(*x, recycler, _fillcount) : x;
            }
            case stringify::v0::alignment::center:
            {
                auto halfcount = _fillcount / 2;
                auto x = _write_fill(buff, recycler, halfcount);
                if(x) x = _write_str(*x, recycler);
                return x ? _write_fill(*x, recycler, _fillcount - halfcount) : x;
            }
            default:
            {
                auto x = _write_fill(buff, recycler, _fillcount);
                return x ? _write_str(*x, recycler) : x;
            }
        }
    }
    return _write_str(buff, recycler);
}

template <typename CharT>
stringify::v0::expected_output_buffer<CharT> fmt_string_printer<CharT>::_write_str
    ( stringify::v0::output_buffer<CharT> buff
    , stringify::buffer_recycler<CharT>& recycler ) const
{
    return stringify::v0::detail::write_str
        ( buff, recycler, _fmt.value().begin(), _fmt.value().length() );
}

template <typename CharT>
stringify::v0::expected_output_buffer<CharT> fmt_string_printer<CharT>::_write_fill
    ( stringify::v0::output_buffer<CharT> buff
    , stringify::buffer_recycler<CharT>& recycler
    , unsigned count ) const
{
    return stringify::v0::detail::write_fill
        ( _encoding, buff, recycler, count, _fmt.fill(), _epoli.err_hdl() );
}

#if defined(BOOST_STRINGIFY_NOT_HEADER_ONLY)

BOOST_STRINGIFY_EXPLICIT_TEMPLATE class string_printer<char>;
BOOST_STRINGIFY_EXPLICIT_TEMPLATE class string_printer<char16_t>;
BOOST_STRINGIFY_EXPLICIT_TEMPLATE class string_printer<char32_t>;
BOOST_STRINGIFY_EXPLICIT_TEMPLATE class string_printer<wchar_t>;

BOOST_STRINGIFY_EXPLICIT_TEMPLATE class fmt_string_printer<char>;
BOOST_STRINGIFY_EXPLICIT_TEMPLATE class fmt_string_printer<char16_t>;
BOOST_STRINGIFY_EXPLICIT_TEMPLATE class fmt_string_printer<char32_t>;
BOOST_STRINGIFY_EXPLICIT_TEMPLATE class fmt_string_printer<wchar_t>;

#endif // defined(BOOST_STRINGIFY_NOT_HEADER_ONLY)

} // namespace detail

template <typename CharOut, typename FPack>
inline stringify::v0::detail::string_printer<CharOut>
make_printer
   ( const FPack& fp
   , const char* str )
{
    static_assert( std::is_same<char, CharOut>::value
                 , "Character type mismatch. Use cv function." );
    return {fp, str, std::char_traits<char>::length(str)};
}

template <typename CharOut, typename FPack>
inline stringify::v0::detail::string_printer<CharOut>
make_printer
   ( const FPack& fp
   , const char16_t* str )
{
    static_assert( std::is_same<char16_t, CharOut>::value
                 , "Character type mismatch. Use cv function." );
    return {fp, str, std::char_traits<char16_t>::length(str)};
}

template <typename CharOut, typename FPack>
inline stringify::v0::detail::string_printer<CharOut>
make_printer
   ( const FPack& fp
   , const char32_t* str )
{
    static_assert( std::is_same<char32_t, CharOut>::value
                 , "Character type mismatch. Use cv function." );
    return {fp, str, std::char_traits<char32_t>::length(str)};
}

template <typename CharOut, typename FPack>
inline stringify::v0::detail::string_printer<CharOut>
make_printer
   ( const FPack& fp
   , const wchar_t* str )
{
    static_assert( std::is_same<wchar_t, CharOut>::value
                 , "Character type mismatch. Use cv function." );
    return {fp, str, std::char_traits<wchar_t>::length(str)};
}

template
    < typename CharOut
    , typename FPack
    , typename CharIn
    , typename Traits
    , typename Allocator >
inline stringify::v0::detail::string_printer<CharOut>
make_printer
   ( const FPack& fp
   , const std::basic_string<CharIn, Traits, Allocator>& str )
{
    static_assert( std::is_same<CharIn, CharOut>::value
                 , "Character type mismatch. Use cv function." );
    return {fp, str.data(), str.size()};
}

template
    < typename CharOut
    , typename FPack
    , typename CharIn
    , typename Traits >
inline stringify::v0::detail::string_printer<CharOut>
make_printer
   ( const FPack& fp
   , const boost::basic_string_view<CharOut, Traits>& str )
{
    static_assert( std::is_same<CharIn, CharOut>::value
                 , "Character type mismatch. Use cv function." );
    return {fp, str.data(), str.size()};
}

#if defined(BOOST_STRINGIFY_HAS_STD_STRING_VIEW)

template
    < typename CharOut
    , typename FPack
    , typename CharIn
    , typename Traits >
inline stringify::v0::detail::string_printer<CharOut>
make_printer
   ( const FPack& fp
   , const std::basic_string_view<CharIn, Traits>& str )
{
    static_assert( std::is_same<CharIn, CharOut>::value
                 , "Character type mismatch. Use cv function." );
    return {fp, str.data(), str.size()};
}

#endif //defined(BOOST_STRINGIFY_HAS_STD_STRING_VIEW)

template <typename CharOut, typename FPack, typename CharIn>
inline stringify::v0::detail::fmt_string_printer<CharOut>
make_printer
   ( const FPack& fp
   , const stringify::v0::string_with_format<CharIn>& input)
{
    static_assert( std::is_same<CharIn, CharOut>::value
                 , "Character type mismatch. Use fmt_cv function." );
    return {fp, input};
}

BOOST_STRINGIFY_V0_NAMESPACE_END

#endif  /* BOOST_STRINGIFY_V0_DETAIL_INPUT_TYPES_CHAR_PTR */
