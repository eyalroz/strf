#ifndef BOOST_STRINGIFY_V0_DISPATCHER_HPP
#define BOOST_STRINGIFY_V0_DISPATCHER_HPP

//  Distributed under the Boost Software License, Version 1.0.
//  (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)

#include <boost/stringify/v0/printer.hpp>
#include <boost/stringify/v0/facets_pack.hpp>
#include <boost/stringify/v0/detail/tr_string.hpp>

BOOST_STRINGIFY_V0_NAMESPACE_BEGIN

template < typename OutbufCreator
         , typename FPack = stringify::v0::facets_pack<> >
class dispatcher_with_given_size;

template < typename OutbufCreator
         , typename FPack = stringify::v0::facets_pack<> >
class dispatcher_calc_size;

template < typename OutbufCreator
         , typename FPack = stringify::v0::facets_pack<> >
class dispatcher_no_reserve;

namespace detail {

#ifdef __cpp_fold_expressions

template <typename ... Printers>
inline std::size_t sum_necessary_size(const Printers& ... printers)
{
    return (... + printers.necessary_size());
}

template <typename CharT, typename ... Printers>
inline void write_args( stringify::v0::basic_outbuf<CharT>& ob
                      , const Printers& ... printers )
{
    (... , printers.write(ob));
}

#else

inline std::size_t sum_necessary_size()
{
    return 0;
}

template <typename Printer, typename ... Printers>
inline std::size_t sum_necessary_size(const Printer& printer, const Printers& ... printers)
{
    return printer.necessary_size()
        + stringify::v0::detail::sum_necessary_size(printers...);
}


template <typename CharT>
inline void write_args(stringify::v0::basic_outbuf<CharT>&)
{
}

template <typename CharT, typename Printer, typename ... Printers>
inline void write_args
    ( stringify::v0::basic_outbuf<CharT>& ob
    , const Printer& printer
    , const Printers& ... printers )
{
    printer.write(ob);
    if (ob.good()) {
        write_args(ob, printers ...);
    }
}

#endif

struct dispatcher_tag {};

// template < typename T
//          , typename = decltype(std::declval<T>().create(std::size_t(0))) >
// std::true_type test_can_rval_ref_create_from_size(T*);

// template < typename T >
// std::false_type test_can_rval_ref_create_from_size(...);
// template < typename T >
// std::false_type test_can_const_create_from_size(...);
// template < typename T >
// std::false_type test_can_rval_ref_create_without_size(...);
// template < typename T >
// std::false_type test_can_const_create_without_size(...);
// std::declval<const T&>().create(std::size_t(0));
/*
template <typename T>
class outbuf_creator_traits
{
    using stripT = std::remove_cv_t<std::remove_reference_t<T>>;

    static T  _val();

    template <typename OB, typename = decltype((OB*)(0)->finish())>
    static constexpr bool _has_finish(OB* ob) -> decltype((OB*)(0)->finish())
    {
        return true;
    }
    template <typename OB>
    static constexpr bool _has_finish(...)
    {
        return false;
    }

    using _sized_outbuf_type = decltype(_val().create(std::size_t{}));
    using _non_sized_outbuf_type = decltype(_val().create());

public:

    template < typename OB
             , std::enable_if_t<_has_finish<OB>(), int> = 0 >
    decltype(auto) finish(OB& ob)
    {
        return ob.finish();
    }

    template < typename OB
             , std::enable_if_t<!_has_finish<OB>(), unsigned> = 0 >
    decltype(auto) finish(OB& ob)
    {
        return stripT::finish(ob);
    }

    using size_finish_type
    = delctype(finish<_sized_outbuf_type>(*(_sized_outbuf_type*)0));

    using non_size_finish_type
    = delctype(finish<_non_sized_outbuf_type>(*(_sized_outbuf_type*)0));
};
*/

// template <typename OutbufCreator>
// class outbuf_creator_traits
// {
// public:
//     constexpr static bool can_const_create_from_size = true;
//     constexpr static bool can_rval_ref_create_from_size = true;
//     constexpr static bool can_const_create_without_size = true;
//     constexpr static bool can_rval_ref_create_without_size = true;

//     static get(//...)
//     static finish(//...)
// };


template < template <typename, typename> class DispatcherTmpl
         , class OutbufCreator
         , class FPack >
class dispatcher_common
{
    using _dispatcher_type = DispatcherTmpl<OutbufCreator, FPack>;
public:

    using finish_type = typename OutbufCreator::finish_type;
    using char_type = typename OutbufCreator::char_type;

    template <typename ... FPE>
    BOOST_STRINGIFY_NODISCARD constexpr auto facets(FPE&& ... fpe) const &
    {
        static_assert( std::is_copy_constructible<OutbufCreator>::value
                     , "OutbufCreator must be copy constructible" );

        const auto& self = static_cast<const _dispatcher_type&>(*this);

        using NewFPack = decltype
            ( stringify::v0::pack( std::declval<const FPack&>()
                                 , std::forward<FPE>(fpe) ...) );

        return DispatcherTmpl<OutbufCreator, NewFPack>
        { self, detail::dispatcher_tag{}, std::forward<FPE>(fpe) ...};
    }

    template <typename ... FPE>
    BOOST_STRINGIFY_NODISCARD constexpr auto facets(FPE&& ... fpe) &&
    {
        static_assert( std::is_move_constructible<OutbufCreator>::value
                     , "OutbufCreator must be move constructible" );

        auto& self = static_cast<const _dispatcher_type&>(*this);

        using NewFPack = decltype
            ( stringify::v0::pack( std::declval<FPack>()
                                 , std::forward<FPE>(fpe) ...) );

        return DispatcherTmpl<OutbufCreator, NewFPack>
        { std::move(self), detail::dispatcher_tag{}, std::forward<FPE>(fpe) ...};
    }

    constexpr stringify::v0::dispatcher_no_reserve<OutbufCreator, FPack>
    no_reserve() const &
    {
        const auto& self = static_cast<const _dispatcher_type&>(*this);
        return { stringify::v0::detail::dispatcher_tag{}
               , self._outbuf_creator
               , self._fpack };
    }

    constexpr stringify::v0::dispatcher_no_reserve<OutbufCreator, FPack>
    no_reserve() &&
    {
        auto& self = static_cast<_dispatcher_type&>(*this);
        return { stringify::v0::detail::dispatcher_tag{}
               , std::move(self._outbuf_creator)
               , std::move(self._fpack) };
    }

    constexpr stringify::v0::dispatcher_calc_size<OutbufCreator, FPack>
    reserve_calc() const &
    {
        const auto& self = static_cast<const _dispatcher_type&>(*this);
        return { stringify::v0::detail::dispatcher_tag{}
               , self._outbuf_creator
               , self._fpack };
    }

    stringify::v0::dispatcher_calc_size<OutbufCreator, FPack>
    reserve_calc() &&
    {
        auto& self = static_cast<_dispatcher_type&>(*this);
        return { stringify::v0::detail::dispatcher_tag{}
               , std::move(self._outbuf_creator)
               , std::move(self._fpack) };
    }

    constexpr stringify::v0::dispatcher_with_given_size<OutbufCreator, FPack>
    reserve(std::size_t size) const &
    {
        const auto& self = static_cast<const _dispatcher_type&>(*this);
        return { stringify::v0::detail::dispatcher_tag{}
               , size
               , self._outbuf_creator
               , self._fpack };
    }

    constexpr stringify::v0::dispatcher_with_given_size<OutbufCreator, FPack>
    reserve(std::size_t size) &&
    {
        auto& self = static_cast<_dispatcher_type&>(*this);
        return { stringify::v0::detail::dispatcher_tag{}
               , size
               , std::move(self._outbuf_creator)
               , std::move(self._fpack) };
    }

    // template <typename ... Args>
    // finish_type operator()(const Args& ... args) const &
    // {
    //     const auto& self = static_cast<const _dispatcher_type&>(*this);
    //     return self._write
    //         (_as_printer_cref(make_printer<char_type, FPack>(self._fpack, args))...);
    // }

    // template <typename ... Args>
    // finish_type operator()(const Args& ... args) &&
    // {
    //     const auto& self = static_cast<const _dispatcher_type&>(*this);
    //     return std::move(self)._write
    //         (_as_printer_cref(make_printer<char_type, FPack>(self._fpack, args))...);
    // }

#if defined(BOOST_STRINGIFY_HAS_STD_STRING_VIEW)

    template <typename ... Args>
    finish_type tr
        ( const std::basic_string_view<char_type>& str
        , const Args& ... args ) const &
    {
       _tr_write(str.begin, str.end(), args...);
    }

    // template <typename ... Args>
    // finish_type tr
    //     ( const std::basic_string_view<char_type>& str
    //     , const Args& ... args ) &&
    // {
    //     return std::move(*this)._tr_write(str.begin, str.end(), args...);
    // }

#else

    template <typename ... Args>
    finish_type tr
        ( const char_type* str
        , const Args& ... args ) const &
    {
        const auto& self = static_cast<const _dispatcher_type&>(*this);
        return self._tr_write
            ( str, str + std::char_traits<char_type>::length(str), args... );
    }

    // template <typename ... Args>
    // finish_type tr
    //     ( const char_type* str
    //     , const Args& ... args ) &&
    // {
    //     return std::move(*this)._tr_write
    //         ( str, str + std::char_traits<char_type>::length(str), args... );
    // }

#endif

private:

    // static inline const stringify::v0::printer<char_type>&
    // _as_printer_cref(const stringify::v0::printer<char_type>& p)
    // {
    //     return p;
    // }

    // static inline const stringify::v0::printer<char_type>*
    // _as_printer_cptr(const stringify::v0::printer<char_type>& p)
    // {
    //      return &p;
    // }


    // template < typename ... Args >
    // finish_type _tr_write( const char_type* str
    //                      , const char_type* str_end
    //                      , const Args& ... args) const &
    // {
    //     const auto& self = static_cast<const _dispatcher_type&>(*this);

    //     using catenc = stringify::v0::encoding_c<char_type>;
    //     using caterr = stringify::v0::tr_invalid_arg_c;
    //     decltype(auto) enc = stringify::v0::get_facet<catenc, void>(self._fpack);
    //     decltype(auto) arg_err = stringify::v0::get_facet<caterr, void>(self._fpack);
    //     return self._tr_write
    //         ( str
    //         , str_end
    //         , { _as_printer_cptr( make_printer<char_type, FPack>
    //                                                  (self._fpack, args))... }
    //         , enc
    //         , arg_err );
    // }

    // template < typename ... Args >
    // finish_type _tr_write( const char_type* str
    //                      , const char_type* str_end
    //                      , const Args& ... args) &&
    // {
    //     auto& self = static_cast<_dispatcher_type&>(*this);

    //     using catenc = stringify::v0::encoding_c<char_type>;
    //     using caterr = stringify::v0::tr_invalid_arg_c;
    //     decltype(auto) enc = stringify::v0::get_facet<catenc, void>(self._fpack);
    //     decltype(auto) arg_err = stringify::v0::get_facet<caterr, void>(self._fpack);
    //     return std::move(self)._tr_write
    //         ( str
    //         , str_end
    //         , { _as_printer_cptr( make_printer<char_type, FPack>
    //                                                  (self._fpack, args))... }
    //         , enc
    //         , arg_err );
    // }
};

}// namespace detail

template < typename OutbufCreator, typename FPack >
class dispatcher_no_reserve
    : private stringify::v0::detail::dispatcher_common
        < dispatcher_no_reserve, OutbufCreator, FPack>
{
    using _common = stringify::v0::detail::dispatcher_common
        < stringify::v0::dispatcher_no_reserve, OutbufCreator, FPack>;

    template < template <typename, typename> class, class, class>
    friend class stringify::v0::detail::dispatcher_common;

public:

    using finish_type = typename OutbufCreator::finish_type;
    using char_type = typename OutbufCreator::char_type;

    template < typename ... Args
             , std::enable_if_t
                 < std::is_constructible<OutbufCreator, Args...>::value
                 , int > = 0 >
    constexpr dispatcher_no_reserve(Args&&... args)
        : _outbuf_creator(std::forward<Args>(args)...)
    {
    }

    template < std::enable_if_t
                 < std::is_copy_constructible<OutbufCreator>::value
                 , int > = 0 >
    constexpr dispatcher_no_reserve( stringify::v0::detail::dispatcher_tag
                                   , const OutbufCreator& oc
                                   , const FPack& fp )
        : _outbuf_creator(oc)
        , _fpack(fp)
    {
    }

    constexpr dispatcher_no_reserve( stringify::v0::detail::dispatcher_tag
                                   , OutbufCreator&& oc
                                   , FPack&& fp )
        : _outbuf_creator(std::move(oc))
        , _fpack(std::move(fp))
    {
    }

    constexpr dispatcher_no_reserve(const dispatcher_no_reserve&) = default;
    constexpr dispatcher_no_reserve(dispatcher_no_reserve&&) = default;

    using _common::facets;
    using _common::tr;
    using _common::reserve_calc;
    using _common::reserve;

    constexpr dispatcher_no_reserve& no_reserve() &
    {
        return *this;
    }
    constexpr const dispatcher_no_reserve& no_reserve() const &
    {
        return *this;
    }
    constexpr dispatcher_no_reserve&& no_reserve() &&
    {
        return std::move(*this);
    }
    constexpr const dispatcher_no_reserve&& no_reserve() const &&
    {
        return std::move(*this);
    }

    template <typename ... Args>
    finish_type operator()(const Args& ... args) const &
    {
        decltype(auto) ob = _outbuf_creator.create();
        stringify::v0::detail::write_args
            ( ob
            , make_printer<char_type, FPack>(_fpack, args)... );
        return OutbufCreator::finish(ob);
    }

private:

    template <class, class>
    friend class dispatcher_no_reserve;

    template < typename OtherFPack
             , typename ... FPE
             , typename = std::enable_if_t
                 < std::is_copy_constructible<OutbufCreator>::value > >
    constexpr dispatcher_no_reserve
        ( const dispatcher_no_reserve<OutbufCreator, OtherFPack>& other
        , detail::dispatcher_tag
        , FPE&& ... fpe )
        : _outbuf_creator(other._outbuf_creator)
        , _fpack(other._fpack, std::forward<FPE>(fpe)...)
    {
    }

    template < typename OtherFPack, typename ... FPE >
    constexpr dispatcher_no_reserve
        ( dispatcher_no_reserve<OutbufCreator, OtherFPack>&& other
        , detail::dispatcher_tag
        , FPE&& ... fpe )
        : _outbuf_creator(std::move(other._outbuf_creator))
        , _fpack(std::move(other._fpack), std::forward<FPE>(fpe)...)
    {
    }

    // template <typename ... Printers>
    // finish_type _write(const Printers& ... printers) const
    // {
    //     decltype(auto) ob = _outbuf_creator.create();
    //     stringify::v0::detail::write_args(ob, printers...);
    //     return OutbufCreator::finish(ob);
    // }

    static inline const stringify::v0::printer<char_type>*
    _as_printer_cptr(const stringify::v0::printer<char_type>& p)
    {
         return &p;
    }

    template < typename ... Args >
    finish_type _tr_write( const char_type* str
                         , const char_type* str_end
                         , const Args& ... args) const &
    {
        decltype(auto) ob = _outbuf_creator.create();

        using catenc = stringify::v0::encoding_c<char_type>;
        using caterr = stringify::v0::tr_invalid_arg_c;
        decltype(auto) enc = stringify::v0::get_facet<catenc, void>(_fpack);
        decltype(auto) arg_err = stringify::v0::get_facet<caterr, void>(_fpack);
        stringify::v0::detail::tr_string_write
            ( str, str_end
            , { _as_printer_cptr
                    ( make_printer<char_type, FPack>(_fpack, args))... }
            , ob, enc, arg_err );

        return OutbufCreator::finish(ob);
        // return self._tr_write
        //     ( str
        //     , str_end
        //     , { _as_printer_cptr( make_printer<char_type, FPack>
        //                                              (self._fpack, args))... }
        //     , enc
        //     , arg_err );
    }



    // finish_type _tr_write
    //     ( const char_type* str
    //     , const char_type* str_end
    //     , std::initializer_list<const stringify::v0::printer<char_type>*> args
    //     , stringify::v0::encoding<char_type> enc
    //     , stringify::v0::tr_invalid_arg arg_err ) const
    // {
    //     decltype(auto) ob = _outbuf_creator.create();

    //     stringify::v0::detail::tr_string_write
    //         ( str, str_end, args, ob, enc, arg_err );

    //     return OutbufCreator::finish(ob);
    // }

    OutbufCreator _outbuf_creator;
    FPack _fpack;
};

template < typename OutbufCreator, typename FPack >
class dispatcher_with_given_size
    : public stringify::v0::detail::dispatcher_common
        < dispatcher_with_given_size, OutbufCreator, FPack>
{
    using _common = stringify::v0::detail::dispatcher_common
        < stringify::v0::dispatcher_with_given_size, OutbufCreator, FPack>;

    template < template <typename, typename> class, class, class>
    friend class stringify::v0::detail::dispatcher_common;

public:

    using finish_type = typename OutbufCreator::finish_type;
    using char_type = typename OutbufCreator::char_type;

    template < typename ... Args
             , std::enable_if_t
                 < std::is_constructible<OutbufCreator, Args...>::value
                 , int > = 0 >
    constexpr dispatcher_with_given_size(std::size_t size, Args&&... args)
        : _size(size)
        , _outbuf_creator(std::forward<Args>(args)...)
    {
    }

    template < std::enable_if_t
                 < std::is_copy_constructible<OutbufCreator>::value
                 , int > = 0 >
    constexpr dispatcher_with_given_size( stringify::v0::detail::dispatcher_tag
                                        , std::size_t size
                                        , const OutbufCreator& oc
                                        , const FPack& fp )
        : _size(size)
        , _outbuf_creator(oc)
        , _fpack(fp)
    {
    }

    constexpr dispatcher_with_given_size( stringify::v0::detail::dispatcher_tag
                                        , std::size_t size
                                        , OutbufCreator&& oc
                                        , FPack&& fp )
        : _size(size)
        , _outbuf_creator(std::move(oc))
        , _fpack(std::move(fp))
    {
    }

    constexpr dispatcher_with_given_size(const dispatcher_with_given_size&) = default;
    constexpr dispatcher_with_given_size(dispatcher_with_given_size&&) = default;

    using _common::facets;
    using _common::tr;
    using _common::reserve_calc;
    using _common::no_reserve;

    constexpr dispatcher_with_given_size& reserve(std::size_t size) &
    {
        _size = size;
        return *this;
    }
    constexpr dispatcher_with_given_size&& reserve(std::size_t size) &&
    {
        _size = size;
        return std::move(*this);
    }

    template <typename ... Args>
    finish_type operator()(const Args& ... args) const &
    {
        decltype(auto) ob = _outbuf_creator.create(_size);
        stringify::v0::detail::write_args
            ( ob
            , make_printer<char_type, FPack>(_fpack, args)... );
        return OutbufCreator::finish(ob);
    }

private:

    template <class, class>
    friend class dispatcher_with_given_size;

    template < typename OtherFPack
             , typename ... FPE
             , typename = std::enable_if_t
                 < std::is_copy_constructible<OutbufCreator>::value > >
    constexpr dispatcher_with_given_size
        ( const dispatcher_with_given_size<OutbufCreator, OtherFPack>& other
        , detail::dispatcher_tag
        , FPE&& ... fpe )
        : _size(other._size)
        , _outbuf_creator(other._outbuf_creator)
        , _fpack(other._fpack, std::forward<FPE>(fpe)...)
    {
    }

    template < typename OtherFPack, typename ... FPE >
    constexpr dispatcher_with_given_size
        ( dispatcher_with_given_size<OutbufCreator, OtherFPack>&& other
        , detail::dispatcher_tag
        , FPE&& ... fpe )
        : _size(other.size)
        , _outbuf_creator(std::move(other._outbuf_creator))
        , _fpack(std::move(other._fpack), std::forward<FPE>(fpe)...)
    {
    }

    // template <typename ... Printers>
    // finish_type _write( stringify::v0::basic_outbuf<char_type>& ob
    //                   , const Printers& ... printers) const
    // {

    // }

    // template <typename ... Printers>
    // finish_type _write(const Printers& ... printers) &&
    // {
    //     decltype(auto) ob = std::move(_outbuf_creator).create(_size);
    //     stringify::v0::detail::write_args(ob, printers...);
    //     return OutbufCreator::finish(ob);
    // }

    static inline const stringify::v0::printer<char_type>*
    _as_printer_cptr(const stringify::v0::printer<char_type>& p)
    {
         return &p;
    }

    template < typename ... Args >
    finish_type _tr_write( const char_type* str
                         , const char_type* str_end
                         , const Args& ... args) const &
    {
        decltype(auto) ob = _outbuf_creator.create(_size);

        using catenc = stringify::v0::encoding_c<char_type>;
        using caterr = stringify::v0::tr_invalid_arg_c;
        decltype(auto) enc = stringify::v0::get_facet<catenc, void>(_fpack);
        decltype(auto) arg_err = stringify::v0::get_facet<caterr, void>(_fpack);
        stringify::v0::detail::tr_string_write
            ( str, str_end
            , { _as_printer_cptr
                    ( make_printer<char_type, FPack>(_fpack, args))... }
            , ob, enc, arg_err );

        return OutbufCreator::finish(ob);
        // return self._tr_write
        //     ( str
        //     , str_end
        //     , { _as_printer_cptr( make_printer<char_type, FPack>
        //                                              (self._fpack, args))... }
        //     , enc
        //     , arg_err );
    }

    std::size_t _size;
    OutbufCreator _outbuf_creator;
    FPack _fpack;
};

template < typename OutbufCreator, typename FPack >
class dispatcher_calc_size
    : public stringify::v0::detail::dispatcher_common
        < dispatcher_calc_size, OutbufCreator, FPack>
{
    using _common = stringify::v0::detail::dispatcher_common
        < stringify::v0::dispatcher_calc_size, OutbufCreator, FPack>;

    template < template <typename, typename> class, class, class>
    friend class stringify::v0::detail::dispatcher_common;

public:

    using finish_type = typename OutbufCreator::finish_type;
    using char_type = typename OutbufCreator::char_type;

    template < typename ... Args
             , std::enable_if_t
                 < std::is_constructible<OutbufCreator, Args...>::value
                 , int > = 0 >
    constexpr dispatcher_calc_size(Args&&... args)
        : _outbuf_creator(std::forward<Args>(args)...)
    {
    }

    template < std::enable_if_t
                 < std::is_copy_constructible<OutbufCreator>::value
                 , int > = 0 >
    constexpr dispatcher_calc_size( stringify::v0::detail::dispatcher_tag
                                  , const OutbufCreator& oc
                                  , const FPack& fp )
        : _outbuf_creator(oc)
        , _fpack(fp)
    {
    }

    constexpr dispatcher_calc_size( stringify::v0::detail::dispatcher_tag
                                  , OutbufCreator&& oc
                                  , FPack&& fp )
        : _outbuf_creator(std::move(oc))
        , _fpack(std::move(fp))
    {
    }

    constexpr dispatcher_calc_size(const dispatcher_calc_size&) = default;
    constexpr dispatcher_calc_size(dispatcher_calc_size&&) = default;

    using _common::facets;
    using _common::tr;
    using _common::no_reserve;
    using _common::reserve;

    constexpr const dispatcher_calc_size & reserve_calc() const &
    {
        return *this;
    }
    constexpr dispatcher_calc_size & reserve_calc() &
    {
        return *this;
    }
    constexpr const dispatcher_calc_size && reserve_calc() const &&
    {
        return std::move(*this);
    }
    constexpr dispatcher_calc_size && reserve_calc() &&
    {
        return std::move(*this);
    }

    template <typename ... Args>
    finish_type operator()(const Args& ... args) const &
    {
        return _write(make_printer<char_type, FPack>(_fpack, args)...);
    }

private:

    template <typename, typename>
    friend class dispatcher_calc_size;

    template < typename OtherFPack
             , typename ... FPE
             , typename = std::enable_if_t
                 < std::is_copy_constructible<OutbufCreator>::value > >
    dispatcher_calc_size
        ( const dispatcher_calc_size<OutbufCreator, OtherFPack>& other
        , detail::dispatcher_tag
        , FPE&& ... fpe )
        : _outbuf_creator(other._outbuf_creator)
        , _fpack(other._fpack, std::forward<FPE>(fpe)...)
    {
    }

    template < typename OtherFPack, typename ... FPE >
    dispatcher_calc_size
        ( dispatcher_calc_size<OutbufCreator, OtherFPack>&& other
        , detail::dispatcher_tag
        , FPE&& ... fpe )
        : _outbuf_creator(std::move(other._outbuf_creator))
        , _fpack(std::move(other._fpack), std::forward<FPE>(fpe)...)
    {
    }

    template <typename ... Printers>
    finish_type _write(const Printers& ... printers) const
    {
        auto size = stringify::v0::detail::sum_necessary_size(printers...);
        decltype(auto) ob = _outbuf_creator.create(size);
        stringify::v0::detail::write_args(ob, printers...);
        return OutbufCreator::finish(ob);
    }

    static inline const stringify::v0::printer<char_type>*
    _as_printer_cptr(const stringify::v0::printer<char_type>& p)
    {
         return &p;
    }

    template < typename ... Args >
    finish_type _tr_write( const char_type* str
                         , const char_type* str_end
                         , const Args& ... args) const &
    {
        using catenc = stringify::v0::encoding_c<char_type>;
        using caterr = stringify::v0::tr_invalid_arg_c;
        decltype(auto) enc = stringify::v0::get_facet<catenc, void>(_fpack);
        decltype(auto) arg_err = stringify::v0::get_facet<caterr, void>(_fpack);
        return _tr_write
            ( str
            , str_end
            , { _as_printer_cptr
                    ( make_printer<char_type, FPack>(_fpack, args))... }
            , enc
            , arg_err );
    }

    finish_type _tr_write
        ( const char_type* str
        , const char_type* str_end
        , std::initializer_list<const stringify::v0::printer<char_type>*> args
        , stringify::v0::encoding<char_type> enc
        , stringify::v0::tr_invalid_arg arg_err ) const
    {
        auto invs = stringify::v0::detail::invalid_arg_size(enc, arg_err);
        auto size = stringify::v0::detail::tr_string_size( str, str_end
                                                         , args, invs );
        decltype(auto) ob = _outbuf_creator.create(size);

        stringify::v0::detail::tr_string_write
            ( str, str_end, args, ob, enc, arg_err );

        return OutbufCreator::finish(ob);
    }

    OutbufCreator _outbuf_creator;
    FPack _fpack;
};


template <typename CharOut, typename FPack, typename Arg>
using printer_impl
= decltype(make_printer<CharOut, FPack>( std::declval<FPack>()
                                       , std::declval<Arg>() ) );

BOOST_STRINGIFY_V0_NAMESPACE_END

#endif  // BOOST_STRINGIFY_V0_DISPATCHER_HPP
