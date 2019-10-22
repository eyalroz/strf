#ifndef BOOST_STRINGIFY_V0_DETAIL_INPUT_TYPES_FACETS_PACK_HPP
#define BOOST_STRINGIFY_V0_DETAIL_INPUT_TYPES_FACETS_PACK_HPP

//  Distributed under the Boost Software License, Version 1.0.
//  (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)

#include <boost/stringify/v0/printer.hpp>
#include <boost/stringify/v0/facets_pack.hpp>
#include <boost/stringify/v0/detail/input_types/join.hpp>

BOOST_STRINGIFY_V0_NAMESPACE_BEGIN

template <typename FPack, typename ... Args>
struct inner_pack_with_args
{
    FPack fp;
    stringify::v0::detail::args_tuple<Args...> args;
};

template <typename FPack>
struct inner_pack
{
    template <typename ... T>
    constexpr inner_pack(T&& ... args)
        : fp{std::forward<T>(args)...}
    {
    }

    FPack fp;

    template <typename ... Args>
    constexpr stringify::v0::inner_pack_with_args<FPack, Args...>
    operator()(const Args& ... args) const
    {
        return stringify::v0::inner_pack_with_args<FPack, Args...>
            { fp
            , stringify::v0::detail::args_tuple<Args...>{args ...} };
    }
};

namespace detail {

template <typename CharT>
class pp_range_printer: public stringify::v0::printer<CharT>
{

    using printer_type = stringify::v0::printer<CharT>;
    using pp_range = stringify::v0::detail::printer_ptr_range<CharT>;

public:

    pp_range_printer(const pp_range& args)
        : m_args(args)
    {
    }

    virtual ~pp_range_printer()
    {
    }

    std::size_t necessary_size() const override
    {
        std::size_t sum = 0;
        for(const auto* arg : m_args)
        {
            sum += arg->necessary_size();
        }
        return sum;
    }

    void write(stringify::v0::basic_outbuf<CharT>& ob) const override
    {
        for(const auto& arg : m_args)
        {
            arg->write(ob);
        }
    }

    int width(int limit) const override
    {
        int sum = 0;
        for(auto it = m_args.begin(); sum < limit && it != m_args.end(); ++it)
        {
            sum += (*it) -> width(limit - sum);
        }
        return sum;
    }

private:

    pp_range m_args;
};


template < typename CharT
         , typename ParentFPack
         , typename ChildFPack
         , typename ... Args >
class facets_pack_printer
    : private stringify::v0::facets_pack<ParentFPack, ChildFPack>
    , private stringify::v0::detail::printers_group
        < CharT
        , stringify::v0::facets_pack<ParentFPack, ChildFPack>
        , Args... >
    , public stringify::v0::detail::pp_range_printer<CharT>
{
    using fmt_group
    = stringify::v0::detail::printers_group
        < CharT
        , stringify::v0::facets_pack<ParentFPack, ChildFPack>
        , Args... >;

    using inner_args
    = stringify::v0::inner_pack_with_args<ChildFPack, Args...>;

public:

    facets_pack_printer( const ParentFPack& parent_fp
                       , const inner_args& args )
        : stringify::v0::facets_pack<ParentFPack, ChildFPack>{parent_fp, args.fp}
        , fmt_group{*this, args.args}
        , stringify::v0::detail::pp_range_printer<CharT>{fmt_group::range()}
    {
    }

    virtual ~facets_pack_printer()
    {
    }
};

template <typename ... F>
constexpr bool are_constrainable_impl()
{
    constexpr std::size_t N = sizeof...(F);
    constexpr bool values[N] = {stringify::v0::is_constrainable_v<F> ...};

    for (std::size_t i = 0; i < N; ++i)
    {
        if( ! values[i])
        {
            return false;
        }
    }
    return true;;
}

template <>
constexpr bool are_constrainable_impl<>()
{
    return true;
}

template <typename ... F>
struct all_are_constrainable
{
    constexpr static bool value
        = stringify::v0::detail::are_constrainable_impl<F...>();
};

} // namespace detail

template <typename ... T>
auto facets(T&& ... args)
    -> stringify::v0::inner_pack
           < decltype(stringify::v0::pack(std::forward<T>(args)...)) >
{
    static_assert
        ( stringify::v0::is_constrainable_v
            < decltype(stringify::v0::pack(std::forward<T>(args)...)) >
        , "All facet categories must be constrainable" );
    return {std::forward<T>(args)...};
}

template < typename CharT
         , typename FPack
         , typename InnerFPack
         , typename ... Args >
inline stringify::v0::detail::facets_pack_printer< CharT
                                                 , FPack
                                                 , InnerFPack
                                                 , Args... >
make_printer( const FPack& fp
            , const stringify::v0::inner_pack_with_args<InnerFPack, Args...>& f )
{
    return {fp, f};
}

BOOST_STRINGIFY_V0_NAMESPACE_END

#endif  // BOOST_STRINGIFY_V0_DETAIL_INPUT_TYPES_FACETS_PACK_HPP

