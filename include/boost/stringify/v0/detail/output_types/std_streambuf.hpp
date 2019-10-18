#ifndef BOOST_STRINGIFY_V0_DETAIL_OUTPUT_TYPES_STD_STREAMBUF_HPP
#define BOOST_STRINGIFY_V0_DETAIL_OUTPUT_TYPES_STD_STREAMBUF_HPP

//  Distributed under the Boost Software License, Version 1.0.
//  (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)

#include <streambuf>
#include <boost/stringify/v0/dispatcher.hpp>
#include <boost/stringify/v0/outbuf.hpp>

BOOST_STRINGIFY_V0_NAMESPACE_BEGIN

template <typename CharT, typename Traits = std::char_traits<CharT> >
class basic_streambuf_writer final: public stringify::v0::basic_outbuf<CharT>
{
public:

    explicit basic_streambuf_writer(std::basic_streambuf<CharT, Traits>& dest_)
        : stringify::v0::basic_outbuf<CharT>(_buf, _buf_size)
        , _dest(dest_)
    {
    }

    basic_streambuf_writer() = delete;
    basic_streambuf_writer(const basic_streambuf_writer&) = delete;

#if defined(BOOST_STRINGIFY_NO_CXX17_COPY_ELISION)

    basic_streambuf_writer(basic_streambuf_writer&& r)
        : basic_streambuf_writer(r._dest)
    {}

#else

    basic_streambuf_writer(basic_streambuf_writer&&) = delete;

#endif

    ~basic_streambuf_writer()
    {
    }

    void recycle() override
    {
        std::streamsize count = this->pos() - _buf;
        this->set_pos(_buf);
        if (this->good())
        {
            auto count_inc = _dest.sputn(_buf, count);
            _count += count_inc;
            this->set_good(count_inc == count);
        }
    }

    struct result
    {
        std::streamsize count;
        bool success;
    };

    result finish()
    {
        std::streamsize count = this->pos() - _buf;
        auto g = this->good();
        this->set_pos(_buf);
        this->set_good(false);
        if (g)
        {
            auto count_inc = _dest.sputn(_buf, count);
            _count += count_inc;
            g = (count_inc == count);
        }
        return {_count, g};
    }

private:

    std::basic_streambuf<CharT, Traits>& _dest;
    std::streamsize _count = 0;
    static constexpr std::size_t _buf_size
        = stringify::v0::min_size_after_recycle<CharT>();
    CharT _buf[_buf_size];
};

using streambuf_writer
    = stringify::v0::basic_streambuf_writer< char
                                           , std::char_traits<char> >;

using wstreambuf_writer
    = stringify::v0::basic_streambuf_writer< wchar_t
                                           , std::char_traits<wchar_t> >;

namespace detail {

template <typename CharT, typename Traits>
class basic_streambuf_writer_factory
{
public:

    using outbuf_type = stringify::v0::basic_streambuf_writer<CharT, Traits>;
    using char_type = CharT;
    using finish_type = typename outbuf_type::result;
        
    basic_streambuf_writer_factory
        ( std::basic_streambuf<CharT, Traits>& dest )
        : _dest(dest)
    {
    }

    basic_streambuf_writer_factory(const basic_streambuf_writer_factory&) = default;

    outbuf_type create() const
    {
        return outbuf_type{_dest};
    }

    static finish_type finish(outbuf_type& w)
    {
        return w.finish();
    }
    
private:

    std::basic_streambuf<CharT, Traits>& _dest;
};


} // namespace detail


template <typename CharT, typename Traits = std::char_traits<CharT> >
inline auto write( std::basic_streambuf<CharT, Traits>& dest )
{
    return stringify::v0::dispatcher_no_reserve
        < stringify::v0::detail::basic_streambuf_writer_factory<CharT, Traits> >
        (dest);
}


template<typename CharT, typename Traits = std::char_traits<CharT> >
inline auto write( std::basic_streambuf<CharT, Traits>* dest )
{
    return stringify::v0::write(*dest);
}

BOOST_STRINGIFY_V0_NAMESPACE_END

#endif  // BOOST_STRINGIFY_V0_DETAIL_OUTPUT_TYPES_STD_STREAMBUF_HPP

