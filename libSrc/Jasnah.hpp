// -*- c++ -*-
#if !defined(JASNAH_H)
/* ==========================================================================
   $File: Jasnah.hpp $
   $Version: 1.0 $
   $Notice: (C) Copyright 2015 Chris Osborne. All Rights Reserved. $
   $License: MIT: http://opensource.org/licenses/MIT $
   ========================================================================== */

#define JASNAH_H
#include "Option.hpp"
#include <functional>
#include <stdexcept>
#include <tuple>

// Set LOG through preproc stuff


namespace Jasnah
{
    // TODO(Chris): Check polution
    namespace JasPH = std::placeholders;

    namespace Impl
    {
        // IndexSeq roughly as per C++14 standard
        // Based on github.com/taocpp/sequences under MIT license
        template <typename T, T...  Indices>
        struct IndexSeq
        {
            typedef T value_type;

            static constexpr
            std::size_t
            Size()
            {
                return sizeof...(Indices);
            }
        };

        template <typename, std::size_t, bool Even>
        struct DoubleLength;

        template <typename T, T... Indices, std::size_t Length>
        struct DoubleLength<IndexSeq<T, Indices...>, Length, false> // even
        {
            using type = IndexSeq<T, Indices..., (Length + Indices)...>;
        };

        template <typename T, T... Indices, std::size_t Length>
        struct DoubleLength<IndexSeq<T, Indices...>, Length, true> // odd
        {
            using type = IndexSeq<T, Indices..., (Length + Indices)..., 2*Length>;
        };

        template <std::size_t Length, typename = void>
        struct MakeIndexSeqImpl;

        template <std::size_t Length>
        struct MakeIndexSeqImpl<Length, typename std::enable_if<Length==0>::type>
        {
            using type = IndexSeq<std::size_t>;
        };

        template <std::size_t Length>
        struct MakeIndexSeqImpl<Length, typename std::enable_if<Length==1>::type>
        {
            using type = IndexSeq<std::size_t, 0>;
        };

        template <std::size_t Length, typename>
        struct MakeIndexSeqImpl
            : DoubleLength<typename MakeIndexSeqImpl<Length/2>::type, Length/2, Length%2 == 1>
        {};

        template <typename Func, typename... Args, std::size_t... Seq>
        inline auto
        UnwrapTupleIntoFn(IndexSeq<std::size_t, Seq...>,
                          const Func& f,
                          const std::tuple<Args...>& fnArgs)
            -> decltype(f(std::get<Seq>(fnArgs)...))
        {
            return f(std::get<Seq>(fnArgs)...);
        }
    }

    template <std::size_t Length>
    using MakeIndexSeq = Impl::MakeIndexSeqImpl<Length>;

    template <typename Func, typename... Args>
    inline auto
    UnwrapTupleIntoFn(const Func& f, const std::tuple<Args...>& fnArgs)
        -> decltype(f(std::declval<Args>()...))
    {
        return Impl::UnwrapTupleIntoFn(typename MakeIndexSeq<sizeof...(Args)>::type(), f, fnArgs);
    }

    template <class Func, typename LeftArgs = std::tuple<>, typename RightArgs = std::tuple<> >
    struct Pipeable
    {
    private:
        Func f;
        LeftArgs left;
        RightArgs right;
    public:

        Pipeable(Func&& f)
            : f(std::forward<Func>(f)),
              left(std::tuple<>()),
              right(std::tuple<>())
        {}

        Pipeable(const Func& f, const LeftArgs& l, const RightArgs& r)
            : f(f),
              left(l),
              right(r)
        {}

        template <typename... Args>
        auto
        operator()(Args&&... fnArgs) const
            -> decltype(UnwrapTupleIntoFn(f, std::tuple_cat(left, std::make_tuple(fnArgs...), right)))
        {
            return UnwrapTupleIntoFn(f, std::tuple_cat(left,
                                                       std::make_tuple(std::forward<Args>(fnArgs)...),
                                                       right));
        }

        template <typename T>
        auto
        LeftCurry(T&& fnArg) const
            -> decltype(Pipeable<Func, decltype(std::tuple_cat(left, std::make_tuple(fnArg))), RightArgs>
                        (f, std::tuple_cat(left, std::make_tuple(fnArg)), right))
        {
            return Pipeable<Func, decltype(std::tuple_cat(left, std::make_tuple(fnArg))), RightArgs>
                (f, std::tuple_cat(left, std::make_tuple(fnArg)), right);
        }

        template <typename T>
        auto
        RightCurry(T&& fnArg) const
            -> decltype(Pipeable<Func, LeftArgs, decltype(std::tuple_cat(right, std::make_tuple(fnArg)))>
                        (f, left, std::tuple_cat(right, std::make_tuple(fnArg))))
        {
            return Pipeable<Func, LeftArgs, decltype(std::tuple_cat(right, std::make_tuple(fnArg)))>
                (f, left, std::tuple_cat(right, std::make_tuple(fnArg)));
        }
    };


    template <typename Func>
    auto
    Piped(Func&& f)
        -> decltype(Pipeable<Func>(std::forward<Func>(f)))
    {
        return Pipeable<Func>(std::forward<Func>(f));
    }

    template <class T>
    using InternalType = typename T::value_type;

// Extension methods for container processing
    namespace Impl
    {
        template <typename T, typename... TArgs, template<typename...>class C, typename F>
        C<T,TArgs...> WhereInContainer(const C<T,TArgs...>& ctr, const F& f)
        {
            C<T,TArgs...> result;
            result.reserve(ctr.size());
            for (const auto& x : ctr)
            {
                if (f(x))
                {
                    result.push_back(x);
                }
            }
            return result;
        }

        template <typename T, typename... TArgs, template <typename...>class C, typename F>
        auto
        MapToContainer(const C<T, TArgs...>& ctr, const F& f)
            -> C<decltype(f(std::declval<T>()))>
        {
            using ResType = decltype(f(std::declval<T>()));
            C<ResType> result;
            result.reserve(ctr.size());
            for (const auto& x : ctr)
            {
                result.push_back(f(x));
            }
            return result;
        }
    }

    struct Where
    {
        template <typename... Args>
        auto operator()(Args&&... args) const
            -> decltype(Impl::WhereInContainer(std::forward<Args>(args)...))
        {
            return Impl::WhereInContainer(std::forward<Args>(args)...);
        }
    };

    struct Map
    {
        template <typename... Args>
        auto operator()(Args&&... args) const
            -> decltype(Impl::MapToContainer(std::forward<Args>(args)...))
        {
            return Impl::MapToContainer(std::forward<Args>(args)...);
        }
    };


    // TODO(Chris): Move to be with the rest of the Option<T> code
    template <typename T>
    struct IsOption
    {
        static constexpr bool value = false;
    };

    template <typename T>
    struct IsOption<Jasnah::Option<T> >
    {
        static constexpr bool value = true;
    };

    // http://stackoverflow.com/a/27885283/3847013
    namespace Impl
    {
        // primary template.
        template<class T>
        struct FnInfo : FnInfo<decltype(&T::operator())>
        {};

        // partial specialization for function type
        template<class R, class... Args>
        struct FnInfo<R(Args...)>
        {
            using result_type = R;
            using argument_types = std::tuple<Args...>;
        };

        // partial specialization for function pointer
        template<class R, class... Args>
        struct FnInfo<R (*)(Args...)>
        {
            using result_type = R;
            using argument_types = std::tuple<Args...>;
        };

        // partial specialization for std::function
        template<class R, class... Args>
        struct FnInfo<std::function<R(Args...)> >
        {
            using result_type = R;
            using argument_types = std::tuple<Args...>;
        };

        // partial specialization for pointer-to-member-function (i.e., operator()'s)
        template<class T, class R, class... Args>
        struct FnInfo<R (T::*)(Args...)>
        {
            using result_type = R;
            using argument_types = std::tuple<Args...>;
        };

        template<class T, class R, class... Args>
        struct FnInfo<R (T::*)(Args...) const>
        {
            using result_type = R;
            using argument_types = std::tuple<Args...>;
        };
    }
}


// #define JASNAH_NO_OPTION_SPEC
#ifndef JASNAH_NO_OPTION_SPEC
#if 0
template <class Data, class Func, bool = Jasnah::IsOption<
                                      typename Jasnah::Impl::FnInfo<Func>::result_type>::value >
constexpr auto
operator|(Data&& x, const Func& f)
    -> decltype(f(std::forward<Data>(x)));

template <class Data, class Func, bool = Jasnah::IsOption<
                                      typename Jasnah::Impl::FnInfo<Func>::result_type>::value >
constexpr auto
operator|(Jasnah::Option<Data>&& x, const Func& f)
    -> decltype(f(std::forward<Data>(*x)));
template <class Data, class Func
          , true
          // typename std::enable_if<
          //                             Jasnah::IsOption<
          //                                 // decltype(std::declval<Func>()
          //                                 //          (std::forward<Data>(std::declval<Data>())))>::value
          //                                 // typename std::decay<
          //                                 //     typename std::result_of<Func(Data&&)>::type>::type
          //                                 // typename Jasnah::Impl::FnInfo<Func>::result_type
          //                                 typename Jasnah::Impl::FnInfo<Func>::result_type
          //                             >::value
              // >::type
>
// template <typename Data, typename Func>
constexpr auto operator|(Jasnah::Option<Data>&& x, const Func& f)
    -> decltype(f(std::forward<Data>(*x)))
{
    typedef decltype(f(std::forward<Data>(*x))) RetType;

    if (!x)
    {

        throw std::invalid_argument("Tried to deref None");
        // if (Jasnah::IsOption<RetType>::value)
        // {
        //     return RetType(Jasnah::None);
        // }
        // else
        //     throw std::invalid_argument("Option stuff");
    }


    return f(std::forward<Data>(*x));
}

template <class Data, class Func, false>
constexpr auto
operator|(Jasnah::Option<Data>&& x, const Func& f)
    -> decltype(f(std::forward<Data>(*x)))
{

    if (!x)
        throw std::invalid_argument("Tried to deref None");
    return f(std::forward<Data>(*x));
}

// template <class Data, class Func, typename std::enable_if<
//                                       !(Jasnah::IsOption<decltype(std::declval<Func>()(std::forward<Data>(std::declval<Data>())))>::value)
//                                       >::type
//                                       >
// auto operator|(Jasnah::Option<Data>&& x, const Func& f)
//     -> decltype(f(std::forward<Data>(*x)))
// {
//     typedef decltype(f(std::forward<Data>(*x))) RetType;

//     if (!x)
//     {
//         throw std::invalid_argument("Use Option everywhere...");
//         // if (Jasnah::IsOption<RetType>::value)
//         // {
//         //     return RetType(Jasnah::None);
//         // }
//         // else
//         //     throw std::invalid_argument("Option stuff");
//     }


//     return f(std::forward<Data>(*x));
// }
#else
template <class Data, class Func, typename U>
constexpr Jasnah::Option<U>
operator|(Jasnah::Option<Data>&&x, const Func& f)
{
    if (!x)
        return Jasnah::None;
    return f(std::forward<Data>(*x));
}

template <class Data, class Func>
constexpr auto
operator|(Jasnah::Option<Data>&& x, const Func& f)
    -> decltype(f(std::forward<Data>(*x)))
{
    if (!x)
        throw std::invalid_argument("Deref'd None into non-optional function");
    return f(std::forward<Data>(*x));
}

#endif

#endif

// NOTE(Chris): Operators need to be in the main namespace for this sorta thing
template <class Data, class Func>
constexpr auto
operator|(Data&& x, const Func& f)
    -> decltype(f(std::forward<Data>(x)))
{
    return f(std::forward<Data>(x));
}

// Curry operators
template<typename Func, typename FnArg>
constexpr auto
operator<<(const Func& f, FnArg&& fnArg)
    -> decltype(f.template RightCurry<FnArg>(std::forward<FnArg>(fnArg)))
{
    return f.template RightCurry<FnArg>(std::forward<FnArg>(fnArg));
}

template<typename Func, typename FnArg>
constexpr auto
operator>>(FnArg&& fnArg, const Func& f)
    -> decltype(f.template LeftCurry<FnArg>(std::forward<FnArg>(fnArg)))
{
    return f.template LeftCurry<FnArg>(std::forward<FnArg>(fnArg));
}

#endif
