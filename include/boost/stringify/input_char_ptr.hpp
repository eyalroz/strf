#ifndef BOOST_STRINGIFY_INPUT_CHAR_PTR
#define BOOST_STRINGIFY_INPUT_CHAR_PTR

#include <algorithm>
#include <limits>
#include <boost/stringify/input_base.hpp>

namespace boost
{
namespace stringify
{

template<typename CharT, typename Output, typename Formatting>
struct input_char_ptr: boost::stringify::input_base<CharT, Output, Formatting>
{
    typedef const CharT* input_type;

    typedef boost::stringify::input_base<CharT, Output, Formatting> base;
    
    typedef
        boost::stringify::width_accumulator<Formatting, input_type, CharT>
        width_accumulator_type;
    
public:
   
    input_char_ptr() noexcept
        : str(0)
        , len(0)
    {
    }

    input_char_ptr(const CharT* _str) noexcept
        : str(_str)
        , len((std::numeric_limits<std::size_t>::max) ())
    {
    }

    void set(const CharT* _str) noexcept
    {
        str = _str;
        len = (std::numeric_limits<std::size_t>::max) ();
    }

    virtual std::size_t length(const Formatting& fmt) const noexcept override
    {
        return str_length() + fill_length(fmt);
    }

    void write
        ( Output& out
        , const Formatting& fmt
        ) const noexcept(base::noexcept_output) override
    {
        auto fw = fill_width(fmt);
        if (fw > 0)
        {
            boost::stringify::write_fill<CharT, input_type>(fw, out, fmt);
        }

        if(str)
        {
            out.put(str, str_length());
        }
    }


private:
    const CharT* str;
    mutable std::size_t len;

    std::size_t str_length() const noexcept
    {
        if (len == (std::numeric_limits<std::size_t>::max) ())
        {
            len = std::char_traits<CharT>::length(str);
        }
        return len;
    }

    std::size_t fill_length(const Formatting& fmt) const noexcept
    {
        auto fw = fill_width(fmt);
        if (fw > 0)
        {
            return boost::stringify::fill_length<CharT, input_type>(fw, fmt);
        }
        return 0;
    }

    boost::stringify::width_t fill_width(const Formatting& fmt) const noexcept
    {
        auto total_width = boost::stringify::get_width<input_type>(fmt);
        if(total_width > 0)
        {
            width_accumulator_type acc;
            if(acc.add(str, str_length(), total_width))
            {
                boost::stringify::width_t nonfill_width = acc.result();
                if (nonfill_width < total_width)
                {
                    return total_width - nonfill_width;
                }
            }
        }
        return 0;
    }
}; 

template <typename CharT, typename Output, typename Formatting>
inline
boost::stringify::input_char_ptr<CharT, Output, Formatting>
argf(const CharT* str) noexcept
{
    return str;
}


} // namespace stringify
} // namespace boost



#endif  /* BOOST_STRINGIFY_INPUT_CHAR_PTR */

