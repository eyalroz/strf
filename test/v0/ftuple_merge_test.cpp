//  Distributed under the Boost Software License, Version 1.0.
//  (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)

#include "test_utils.hpp"
#include <boost/detail/lightweight_test.hpp>
#include <boost/stringify.hpp>
#include <vector>

namespace strf = boost::stringify::v0;

struct facet_category;

struct facet_type
{
    using category = facet_category;
    int value = 0;
};

struct facet_category
{
    static const auto& get_default() noexcept
    {
        static const facet_type f {};
        return f;
    }
};

template <int I> struct input_type
{
    static constexpr int type_n_id = I;
};

template<typename T, int N> struct filter_le
{
    static constexpr bool value = (T::type_n_id <= N);
};


template<typename T> using filter_le1 = filter_le<T, 1>;
template<typename T> using filter_le2 = filter_le<T, 2>;
template<typename T> using filter_le3 = filter_le<T, 3>;
template<typename T> using filter_le4 = filter_le<T, 4>;
template<typename T> using filter_le5 = filter_le<T, 5>;
template<typename T> using filter_le6 = filter_le<T, 6>;
template<typename T> using filter_le7 = filter_le<T, 7>;


facet_type f1 = {1};
facet_type f2 = {2};
facet_type f3 = {3};
facet_type f4 = {4};
facet_type f5 = {5};
facet_type f6 = {6};
facet_type f7 = {7};


auto x1 = strf::constrain<filter_le1>(f1);
auto x2 = strf::constrain<filter_le2>(std::ref(f2));
auto x3 = strf::constrain<filter_le3>(std::cref(f3));
auto x4_ = strf::constrain<filter_le4>(f4);
auto x5_ = strf::constrain<filter_le5>(f5);
auto x6_ = strf::constrain<filter_le6>(std::ref(f6));
auto x7_ = strf::constrain<filter_le7>(std::cref(f7));
auto x4 = std::ref (x4_);
auto x5 = std::cref(x5_);
auto x6 = std::ref (x6_);
auto x7 = std::cref(x7_);

template <typename FTuple>
std::vector<int> digest(const FTuple& fmt)
{
    return std::vector<int>
    {
        strf::get_facet< facet_category, input_type<1> >(fmt).value,
        strf::get_facet< facet_category, input_type<2> >(fmt).value,
        strf::get_facet< facet_category, input_type<3> >(fmt).value,
        strf::get_facet< facet_category, input_type<4> >(fmt).value,
        strf::get_facet< facet_category, input_type<5> >(fmt).value,
        strf::get_facet< facet_category, input_type<6> >(fmt).value,
        strf::get_facet< facet_category, input_type<7> >(fmt).value
    };
}

std::vector<int> expected = {1, 2, 3, 4, 5, 6, 7};


int main()
{

    {
        auto fmt = strf::make_ftuple(x7, x6, x5, x4, x3, x2, x1);
        BOOST_TEST(digest(fmt) == expected);
    }

    {
        auto fmt = strf::make_ftuple(x7, x6, x5, x5, x4, x5, x4, x3, x2, x1, x1);
        BOOST_TEST(digest(fmt) == expected);
    }

    {
        auto fmt = strf::make_ftuple
            (strf::make_ftuple(x7), strf::make_ftuple(x6, x5), x4, x3, x2, x1);
        BOOST_TEST(digest(fmt) == expected);
    }

    {
        auto fmt = strf::make_ftuple
            (x7, x6, x5, x4, x3, x2, strf::make_ftuple(x1));
        BOOST_TEST(digest(fmt) == expected);
    }

    {
        auto fmt = strf::make_ftuple
            (x7, x6, x5, strf::make_ftuple(x4, x3), x2, x1);
        BOOST_TEST(digest(fmt) == expected);
    }

    {
        auto fmt = strf::make_ftuple
            ( strf::make_ftuple(x7)
            , strf::make_ftuple(x6)
            , strf::make_ftuple(x5)
            , strf::make_ftuple(x4)
            , strf::make_ftuple(x3)
            , strf::make_ftuple(x2)
            , strf::make_ftuple(x1)
            );
        BOOST_TEST(digest(fmt) == expected);
    }
    {
        auto fmt = strf::make_ftuple
            ( strf::make_ftuple(strf::make_ftuple(x7))
            , strf::make_ftuple(strf::make_ftuple(x6))
            , strf::make_ftuple(strf::make_ftuple(x5))
            , strf::make_ftuple(strf::make_ftuple(x4))
            , strf::make_ftuple(strf::make_ftuple(x3))
            , strf::make_ftuple(strf::make_ftuple(x2))
            , strf::make_ftuple(strf::make_ftuple(x1))
            );
        BOOST_TEST(digest(fmt) == expected);
    }
    {
        auto fmt = strf::make_ftuple
            ( strf::make_ftuple(strf::make_ftuple(x7), strf::make_ftuple(x6))
            , strf::make_ftuple(strf::make_ftuple(x5), strf::make_ftuple(x4))
            , strf::make_ftuple(strf::make_ftuple(x3), strf::make_ftuple(x2))
            , strf::make_ftuple(strf::make_ftuple(x2), strf::make_ftuple(x1))
            );
        BOOST_TEST(digest(fmt) == expected);
    }
    {
        auto fmt = strf::make_ftuple
            ( strf::make_ftuple(strf::make_ftuple(x7, x6, x5))
            , strf::make_ftuple(x6, x5, x4)
            , x4, x3, x2
            , strf::make_ftuple(strf::make_ftuple(x2, x1))
            );
        BOOST_TEST(digest(fmt) == expected);
    }
    {
        auto fmt = strf::make_ftuple
            ( strf::make_ftuple()
            , strf::make_ftuple(strf::make_ftuple(x7, x6))
            , strf::make_ftuple(x5, x4, x5, x4)
            , strf::make_ftuple(x5, x4)
            , x3
            , x2
            , strf::make_ftuple(strf::make_ftuple(x2, x1))
            , strf::make_ftuple()
            );

        BOOST_TEST(digest(fmt) == expected);
    }


    return report_errors() || boost::report_errors();
}


