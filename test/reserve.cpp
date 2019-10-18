//  Distributed under the Boost Software License, Version 1.0.
//  (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)

#define  _CRT_SECURE_NO_WARNINGS

#include "test_utils.hpp"
#include <boost/stringify.hpp>

namespace strf = boost::stringify::v0;

class reservation_tester : public strf::basic_outbuf<char>
{
    constexpr static std::size_t _buff_size = strf::min_size_after_recycle<char>();
    char _buff[_buff_size];

public:

    reservation_tester()
        : strf::basic_outbuf<char>{ _buff, _buff + _buff_size }
        , _buff{0}
    {
    }

    reservation_tester(std::size_t size)
        : strf::basic_outbuf<char>{ _buff, _buff + _buff_size }
        , _buff{0}
        , _reserved_size{size}
    {
    }

    reservation_tester(const reservation_tester&) = delete;

#if defined(BOOST_STRINGIFY_NO_CXX17_COPY_ELISION)

    reservation_tester(reservation_tester&& r)
        : reservation_tester(r._reserved_size)
    {
    }

#else

    reservation_tester(reservation_tester&&) = delete;

#endif
    
    void recycle() override
    {
        this->set_pos(_buff);
    }

    std::size_t finish()
    {
        return _reserved_size;
    }

private:

    std::size_t _reserved_size = 0;
};


class reservation_tester_factory
{
public:

    using char_type = char;
    using outbuf_type = reservation_tester;
    using finish_type = std::size_t;

    static outbuf_type create()
    {
        return {};
    }

    static auto create(std::size_t s)
    {
        return outbuf_type{s};
    }

    static auto finish(outbuf_type& s)
    {
        return s.finish();
    }
};

constexpr auto reservation_test()
{
    return strf::dispatcher_no_reserve<reservation_tester_factory>();
}


int main()
{
    // on non-const rval ref
    constexpr std::size_t not_reserved = 0;

    {
        auto size = reservation_test()  ("abcd");
        BOOST_TEST_EQ(size, not_reserved);
    }
    {
        auto size = reservation_test() .reserve(5555) ("abcd");
        BOOST_TEST_EQ(size, 5555);
    }
    {
        auto size = reservation_test() .reserve_calc() ("abcd");
        BOOST_TEST_EQ(size, 4);
    }

    // // on non-const ref

    {
        auto tester = reservation_test();
        auto size = tester ("abcd");
        BOOST_TEST_EQ(size, not_reserved);
    }
    {
        auto tester = reservation_test();
        auto size = tester.reserve(5555) ("abcd");
        BOOST_TEST_EQ(size, 5555);
    }
    {
        auto tester = reservation_test();
        auto size = tester.reserve_calc() ("abcd");
        BOOST_TEST_EQ(size, 4);
    }

    // // on const ref

    {
        const auto tester = reservation_test();
        auto size = tester ("abcd");
        BOOST_TEST_EQ(size, not_reserved);
    }
    {
        const auto tester = reservation_test();
        auto size = tester.reserve(5555) ("abcd");
        BOOST_TEST_EQ(size, 5555);
    }
    {
        const auto tester = reservation_test() .reserve(5555);
        auto size = tester.reserve_calc() ("abcd");
        BOOST_TEST_EQ(size, 4);
    }

    // // on const rval ref

    {
        const auto tester = reservation_test();
        auto size = std::move(tester) ("abcd");
        BOOST_TEST_EQ(size, not_reserved);
    }
    {
        const auto tester = reservation_test();
        auto size = std::move(tester).reserve(5555) ("abcd");
        BOOST_TEST_EQ(size, 5555);
    }
    {
        const auto tester = reservation_test() .reserve(5555);
        auto size = std::move(tester).reserve_calc() ("abcd");
        BOOST_TEST_EQ(size, 4);
    }

    return boost::report_errors();
}
