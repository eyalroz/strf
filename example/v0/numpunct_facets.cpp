#include <boost/assert.hpp>
#include <boost/stringify.hpp>

void sample1()
{
    //[str_grouping
    namespace strf = boost::stringify::v0;

    {
        constexpr int base = 10;
        auto punct = strf::str_grouping<base>{"\4\3\2"};
        auto str = strf::make_string.with(punct) &= { 100000000000ll };
        BOOST_ASSERT(str == "1,00,00,000,0000");
    }

    {
        auto punct = strf::str_grouping<10>{std::string{"\3\2\0", 3}};
        auto str = strf::make_string.with(punct) &= { 100000000000ll };
        BOOST_ASSERT(str == "1000000,00,000");
    }
    //]
}


void sample2()
{
    //[monotonic_grouping
    namespace strf = boost::stringify::v0;

    unsigned long long value = 0xfffffffffLLU;

    auto str = strf::make_string
        .with(strf::monotonic_grouping<8> {6}.thousands_sep(U':'))
        .with(strf::monotonic_grouping<10>{3}.thousands_sep(U'.'))
        .with(strf::monotonic_grouping<16>{4}.thousands_sep(U'\''))
        ["{} / {} / {}"]
        &= {value, strf::hex(value), strf::oct(value)};

    BOOST_ASSERT(str == "68.719.476.735 / f'ffff'ffff / 777777:777777");
    //]
}

int main()
{
    sample1();
    sample2();

    return 0;
}
