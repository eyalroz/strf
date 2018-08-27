//  Distributed under the Boost Software License, Version 1.0.
//  (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)

#include <vector>

//[ ipv4_addr_type
namespace xxx {
struct ipv4_addr
{
    unsigned char bytes[4];
};
}
//]


//[ make_printer_ipv4_addr
#include <boost/stringify.hpp>

namespace strf = boost::stringify::v0;

namespace xxx {

template <typename CharT, typename FPack>
auto make_printer(strf::output_writer<CharT>& out, const FPack& fp, ipv4_addr addr)
{
    (void)fp;

    auto j = strf::join() ( addr.bytes[0], static_cast<CharT>('.')
                          , addr.bytes[1], static_cast<CharT>('.')
                          , addr.bytes[2], static_cast<CharT>('.')
                          , addr.bytes[3] );

    return make_printer(out, /*<< Should we pass `fp` here instead of an empty facets pack?
The aswer is no because it could yield to an incorrect IPv4 representation.
Consider, for example, if numeric punctuation were applied. Here we just want to use the
default facets. In other input types, however, you may decide to propagate
some, or all of the facets.
>>*/strf::pack(), j);
}

} // namespace xxx


//]

void basic_sample()
{
//[ ipv4_basic_sample
    xxx::ipv4_addr addr {{146, 20, 110, 251}};
    auto s1 = strf::to_string("The IP address of boost.org is ", addr).value();
    BOOST_ASSERT(s1 == "The IP address of boost.org is 146.20.110.251");
//]
}


//[fmt_ipv4_addr
namespace xxx {

class fmt_ipv4_addr: public strf::align_formatting<fmt_ipv4_addr>
{
public:

    fmt_ipv4_addr(ipv4_addr a) : addr(a) {}

  /*<< This constructor is only needed if you want to enable the formatting of a range of `ipv4_addr` like this: [table [ [   ] [ [fmt_range_ipv4_addr] ]] ]
>>*/ template <typename U>
    fmt_ipv4_addr(ipv4_addr value, const strf::align_formatting<U>& fmt)
        : strf::align_formatting<fmt_ipv4_addr>(fmt)
        , addr(value)
    {
    }

    void operator%(int) const = delete;

    ipv4_addr addr;
};

}
//]

//[make_fmt_ipv4
namespace xxx {
inline fmt_ipv4_addr make_fmt( /*<< The `tag` paramenter is not used. It is present just to ensure that there is no other `make_fmt` function around there with the same signature. >>*/ strf::tag, ipv4_addr x) { return {x}; }
}
//]


//[make_printer_fmt_ipv4
namespace xxx {

template <typename CharT, typename FPack>
auto make_printer( strf::output_writer<CharT>& out
                        , const FPack& fp
                        , fmt_ipv4_addr fmt_addr )
{
    (void)fp;

    auto j = strf::join(fmt_addr.width(), fmt_addr.alignment(), fmt_addr.fill())
            ( fmt_addr.addr.bytes[0], static_cast<CharT>('.')
            , fmt_addr.addr.bytes[1], static_cast<CharT>('.')
            , fmt_addr.addr.bytes[2], static_cast<CharT>('.')
            , fmt_addr.addr.bytes[3] );

    return strf::make_printer(out, strf::facets_pack<>{}, j);
}

} // namespace xxx
//]

void sample_fmt_sample()
{
    //[ipv4_fmt_sample
    xxx::ipv4_addr addr {{146, 20, 110, 251}};

    auto s2 = strf::to_string("boost.org: ", strf::right(addr, 20, U'.')) .value();
    BOOST_ASSERT(s2 == "boost.org: ......146.20.110.251");
    //]
}


void sample_fmt_range_ipv4_addr()
{
//[fmt_range_ipv4_addr
    std::vector<xxx::ipv4_addr> vec = { {{127, 0, 0, 1}}
                                      , {{146, 20, 110, 251}}
                                      , {{110, 110, 110, 110}} };
    auto s4 = strf::to_string("[", strf::fmt_range(vec) > 16, "]").value();
    BOOST_ASSERT(s4 == "[       127.0.0.1  146.20.110.251 110.110.110.110]");
//]

    auto s3 = strf::to_string("[", strf::range(vec, " ; "), "]").value();
    BOOST_ASSERT(s3 == "[127.0.0.1 ; 146.20.110.251 ; 110.110.110.110]");
}


#include <vector>

int main()
{
    basic_sample();
    sample_fmt_sample();
    sample_fmt_range_ipv4_addr();
    return 0;
}