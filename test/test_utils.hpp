#ifndef STRINGIFY_TEST_TEST_UTILS_HPP_INCLUDED
#define STRINGIFY_TEST_TEST_UTILS_HPP_INCLUDED

#include <boost/detail/lightweight_test.hpp>
#include <boost/stringify.hpp>


template <typename CharT>
struct to_upper_char_traits : public std::char_traits<CharT>
{
  static CharT*
  copy(CharT* to, const CharT* from, std::size_t n)
  {
    CharT* it = to;
    while(n--)
      *it++ = std::toupper(*from++);
    return to;
  }

  static void
  assign(CharT& c1, const CharT& c2)
  {
    c1 = std::toupper(c2);
  }

  static CharT*
  assign(CharT* dest, std::size_t n, CharT a)
  {
    std::fill_n(dest, n, std::toupper(a));
    return dest;
  }
};


template <typename CharT>
struct weird_char_traits : public std::char_traits<CharT>
{
  static CharT*
  copy(CharT* to, const CharT* from, std::size_t n)
  {
    CharT* it = to;
    while(n--)
    {
        assign(*it++, *from++);
    }
    return to;
  }

  static void
  assign(CharT& c1, const CharT& c2)
  {
      if (c2 == CharT())
      {
          c1 = c2;
      }
      else
      {
          c1 = c2 | ( 1 << (sizeof(CharT) * 8 - 1));
      }
  }

  static CharT*
  assign(CharT* dest, std::size_t n, CharT a)
  {
      CharT b;
      assign(b, a);
      std::fill_n(dest, n, b);
      return dest;
  }
};


template <int LINE_NUM, typename CharT>
class tester
{
    typedef
    boost::stringify::detail::char_ptr_writer<CharT, std::char_traits<CharT> >
    writer_type;
    
public:
    typedef CharT char_type;
    
    tester(std::basic_string<CharT> expected)
        : m_expected_size(0)
        , m_expected(std::move(expected))
    {
    }

    tester(tester&&) = default;
    
    ~tester()
    {
    }
          
    void reserve(std::size_t size)
    {
        m_writer.reserve(size);
        m_expected_size = size;
    }
    
    bool finish()
    {
        auto result = m_writer.finish();
        BOOST_TEST(m_expected == result);
        BOOST_TEST(m_expected_size == 1 + m_expected.length());
        return m_expected == result
            && m_expected_size == 1 + m_expected.length();
    }

    void put(CharT character) noexcept
    {
        m_writer.put(character);
    }

    void put(CharT character, std::size_t repetitions) noexcept
    {
        m_writer.put(character, repetitions);
    }

    void put(const CharT* str, std::size_t count) noexcept
    {
        m_writer.put(str, count);
    }
    
    bool set_pos(std::size_t pos) noexcept
    {
        return m_writer.set_pos(pos);
    }

    decltype(auto) get_pos() noexcept
    {
        return m_writer.get_pos();
    }
    
    void rput(CharT character) noexcept
    {
        m_writer.rput(character);
    }

private:

    std::size_t m_expected_size;
    std::basic_string<CharT> m_expected;
    boost::stringify::detail::string_maker<std::basic_string<CharT>> m_writer;
};
      
template<int LINE_NUM, typename CharT>
decltype(auto)
    testf(const CharT* expected)    
{
    return boost::stringify::writef_helper<tester<LINE_NUM, CharT>>(expected);
}



#endif


