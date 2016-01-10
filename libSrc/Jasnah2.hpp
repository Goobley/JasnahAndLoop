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

        // template <std::size_t Length, typename = void>
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


#ifdef JASNAH_PIPE_ORIG
    template<class F>
    struct Pipeable
    {
    private:
        F func;
    public:
        Pipeable(F&& f)
            : func(std::forward<F>(f))
        {}

        template <class... Vals>
        auto operator()(Vals&&... vals)
            -> decltype(std::bind(func, JasPH::_1, std::forward<Vals>(vals)...))
        {
            return std::bind(func, JasPH::_1, std::forward<Vals>(vals)...);
        }
    };

    template <class F>
    Pipeable<F> Piped(F&& f)
    {
        return Pipeable<F>(std::forward<F>(f));
    }
#else
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

    // namespace Impl
    // {
    //     template <typename Indices, typename Func, typename... Args>
    //     struct PartialFunc;

    //     struct MakePartialFunc
    //     {
    //         struct Secret {};
    //         template <typename Func, typename... Args>
    //         constexpr PartialFunc<typename Jasnah::MakeIndexSeq<sizeof...(Args)>::type,
    //                               typename std::decay<Func>::type,
    //                               typename std::decay<Args>::type...>
    //         operator()(Func&& f, Args&&... args) const
    //         {
    //             return {Secret{}, static_cast<Func&&>(f), static_cast<Args&&>(args)...};
    //         }
    //     };

    //     template <std::size_t... Indices, typename Func, typename... Args>
    //     // struct PartialFunc<Impl::IndexSeq<std::size_t, Indices...>, Func, Args...>
    //     struct PartialFunc<Impl::IndexSeq<std::size_t, Indices...>, Func, Args...>
    //     {
    //         PartialFunc() = default;
    //         std::tuple<Func, Args...> storage;

    //         template <typename... X>
    //         constexpr PartialFunc(MakePartialFunc::Secret, X&&... args)
    //             : storage{static_cast<X&&>(args)...}
    //         {
    //         }

    //         // const lval
    //         template <typename... X>
    //         constexpr auto
    //         operator()(X&&... fnArgs) const&
    //             -> decltype(std::get<0>(storage)
    //                         (std::get<Indices+1>(storage)...,
    //                          static_cast<X&&>(fnArgs)...))
    //         {
    //             return std::get<0>(storage)
    //                 (std::get<Indices+1>(storage)...,
    //                  static_cast<X&&>(fnArgs)...);
    //         }

    //         // non-const lval C++11 problems
    //         template <typename... X>
    //         constexpr auto
    //         operator()(X&&... fnArgs) &
    //             -> decltype(std::get<0>(storage)
    //                         (std::get<Indices+1>(storage)...,
    //                          static_cast<X&&>(fnArgs)...))
    //         {
    //             return std::get<0>(storage)
    //                 (std::get<Indices+1>(storage)...,
    //                  static_cast<X&&>(fnArgs)...);
    //         }

    //         // rval
    //         template <typename... X>
    //         constexpr auto
    //         operator()(X&&... fnArgs) &&
    //             -> decltype(static_cast<Func&&>(std::get<0>(storage))
    //                         (static_cast<Args&&>(std::get<Indices+1>(storage))...,
    //                          static_cast<X&&>(fnArgs)...))
    //         {
    //             return static_cast<Func&&>(std::get<0>(storage))
    //                 (static_cast<Args&&>(std::get<Indices+1>(storage))...,
    //                  static_cast<X&&>(fnArgs)...);
    //         }
    //     };

    //     constexpr MakePartialFunc PartialFn{};

    //     struct ApplyToFunc
    //     {
    //         // Default method
    //         template <typename Func, typename... Args>
    //         constexpr auto operator()(Func&& f, Args&&... args) const
    //             -> decltype(static_cast<Func&&>(f)(static_cast<Args&&>(args)...))
    //         {
    //             return static_cast<Func&&>(f)(static_cast<Args&&>(args)...);
    //         }

    //         // Handles virtual function calls
    //         template <typename Base, typename T, typename Derived>
    //         constexpr auto operator()(T Base::*pmd, Derived&& ref) const
    //             -> decltype(static_cast<Derived&&>(ref).*pmd)
    //         {
    //             return static_cast<Derived&&>(ref).*pmd;
    //         }

    //         // Pointer to member data
    //         template <typename PMD, typename Ptr>
    //         constexpr auto operator()(PMD pmd, Ptr&& ptr) const
    //             -> decltype((*static_cast<Ptr&&>(ptr)).*pmd)
    //         {
    //             return (*static_cast<Ptr&&>(ptr)).*pmd;
    //         }

    //         // Pointer to member function from class instance
    //         template <typename Base, typename T, typename Derived, typename... Args>
    //         constexpr auto operator()(T Base::*pmf, Derived&& ref, Args&&... args) const
    //             -> decltype((static_cast<Derived&&>(ref).*pmf)(static_cast<Args&&>(args)...))
    //         {
    //             return (static_cast<Derived&&>(ref).*pmf)(static_cast<Args&&>(args)...);
    //         }

    //         // Pointer to member function from class pointer
    //         template <typename PMF, typename Ptr, typename... Args>
    //         constexpr auto operator()(PMF pmf, Ptr&& ptr, Args&&... args) const
    //             -> decltype(((*static_cast<Ptr&&>(ptr)).*pmf)(static_cast<Args&&>(args)...))
    //         {
    //             return ((*static_cast<Ptr&&>(ptr)).*pmf)(static_cast<Args&&>(args)...);
    //         }
    //     };

    //     constexpr ApplyToFunc ApplyFn{};


    // }
    // // Based on boost::hana
    // template <std::size_t NArgs, typename Func>
    // struct Curry;

    // template <std::size_t NArgs>
    // struct MakeCurry
    // {
    //     template <typename Func>
    //     constexpr Curry<NArgs, typename std::decay<Func>::type>
    //     operator()(Func&& f) const
    //     {
    //         return {static_cast<Func&&>(f)};
    //     }
    // };

    // template <std::size_t NArgs>
    // constexpr MakeCurry<NArgs> curry{};

    // #if 0
    // template <std::size_t NArgs>
    // struct CurryOrCall;

    // template <std::size_t NArgs>
    // struct CurryOrCall
    // {
    //     static constexpr MakeCurry<NArgs> Impl{};
    // };

    // template <>
    // struct CurryOrCall<0>
    // {
    //     static constexpr auto Impl = Impl::ApplyFn;
    // };
    // #else
    // template <std::size_t N>
    // constexpr MakeCurry<N> CurryOrCall{};

    // template <>
    // constexpr auto CurryOrCall<0> = Impl::ApplyFn;
    // #endif

    // template <std::size_t NArgs, typename Func>
    // struct Curry
    // {
    //     Func f;

    //     template <typename... Args>
    //     constexpr auto
    //     operator()(Args&&... args) const &
    //         -> decltype(CurryOrCall<NArgs - sizeof...(Args)>
    //                     (Impl::PartialFn(f, static_cast<Args&&>(args)...)))
    //     {
    //         static_assert(sizeof...(args) <= NArgs, "Too many args for Jasnah::Curry");
    //         return CurryOrCall<NArgs - sizeof...(Args)>
    //             (Impl::PartialFn(f, static_cast<Args&&>(args)...));
    //     }

    //     template <typename... Args>
    //     constexpr auto
    //     operator()(Args&&... args) &
    //         -> decltype(CurryOrCall<NArgs - sizeof...(Args)>
    //                     (Impl::PartialFn(f, static_cast<Args&&>(args)...)))
    //     {
    //         static_assert(sizeof...(args) <= NArgs, "Too many args for Jasnah::Curry");
    //         return CurryOrCall<NArgs - sizeof...(Args)>
    //             (Impl::PartialFn(f, static_cast<Args&&>(args)...));
    //     }

    //     template <typename... Args>
    //     constexpr auto
    //     operator()(Args&&... args) &&
    //         -> decltype(CurryOrCall<NArgs - sizeof...(Args)>
    //                     (Impl::PartialFn(std::move(f), static_cast<Args&&>(args)...)))
    //     {
    //         static_assert(sizeof...(args) <= NArgs, "Too many args for Jasnah::Curry");
    //         return CurryOrCall<NArgs - sizeof...(Args)>
    //             (Impl::PartialFn(std::move(f), static_cast<Args&&>(args)...));
    //     }

    // };

    // template <typename Func>
    // struct Curry<0, Func>
    // {
    //     Func f;

    //     constexpr auto
    //     operator()() const &
    //         -> decltype(f())
    //     {
    //         return f();
    //     }

    //     constexpr auto
    //     operator()() &
    //         -> decltype(f())
    //     {
    //         return f();
    //     }

    //     constexpr auto
    //     operator()() &&
    //         -> decltype(std::move(f)())
    //     {
    //         return std::move(f)();
    //     }
    // };

    // // L&R curry, currently we only have left curry

#endif

    template <class T>
    using InternalType = typename T::value_type;

// Extension methods for container processing
// TODO(Chris): These use push_back for now, fix
// TODO(Chris): Should probably reserve or something
    // template <typename T, typename F>
    // T WhereInContainer(const T& cont, const F& Property)
    // // T WhereInContainer(const T& cont, std::function<bool(InternalType<T>)> Property)
    // {
    //     T result;
    //     result.reserve(cont.size());
    //     for (const auto& x : cont)
    //     {
    //         if (Property(x))
    //         {
    //             result.push_back(x);
    //         }
    //     }
    //     return result;
    // }
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

    struct WhereImpl
    {
        template <typename... Args>
        auto operator()(Args&&... args) const
            -> decltype(WhereInContainer(std::forward<Args>(args)...))
        {
            return WhereInContainer(std::forward<Args>(args)...);
        }
    };

    // template <class T, typename F>
    // T MapToContainer(const T& cont, const F& Func)
    // // T MapToContainer(const T& cont, std::function<InternalType<T>
    // //                  (InternalType<T>)> Func)
    // {
    //     T result;
    //     result.reserve(cont.size());
    //     for (const auto& x : cont)
    //     {
    //         result.push_back(Func(x));
    //     }
    //     return result;
    // }

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

    struct MapImpl
    {
        template <typename... Args>
        auto operator()(Args&&... args) const
            -> decltype(MapToContainer(std::forward<Args>(args)...))
        {
            return MapToContainer(std::forward<Args>(args)...);
        }
    };

#ifdef JASNAH_ORIG_PIPE
    template<typename T>
    auto MakeFnPipeableOneArg(T&& t) -> decltype(Piped(std::bind(t, JasPH::_1)))
    {
        return Piped(std::bind(t, JasPH::_1));
    }

    template<typename T>
    auto MakeFnPipeableTwoArgs(T&& t) -> decltype(Piped(std::bind(t, JasPH::_1, JasPH::_2)))
    {
        return Piped(std::bind(t, JasPH::_1, JasPH::_2));
    }
#else
    template<typename T>
    auto MakeFnPipeableOneArg(T&& t) -> decltype(Piped(t))
    {
        return Piped(t);
    }

    template<typename T>
    auto MakeFnPipeableTwoArgs(T&& t) -> decltype(Piped(t))
    {
        return Piped(t);
    }
#endif

    //NOTE:: Export funcs, make more prominent
    // template <class T>
    // constexpr auto
    // WhereFor()
    //     -> decltype(MakeFnPipeableTwoArgs(WhereInContainer<T>))
    // {
    //     return MakeFnPipeableTwoArgs(WhereInContainer<T>);
    // }

    // template <class T>
    // constexpr auto
    // MapFor()
    //     -> decltype(MakeFnPipeableTwoArgs(MapToContainer<T>))
    // {
    //     return MakeFnPipeableTwoArgs(MapToContainer<T>);
    // }

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
}

// NOTE(Chris): Operators need to be in the main namespace for this sorta thing
template <class Data, class Func>
auto operator|(Data&& x, const Func& f)
    -> decltype(f(std::forward<Data>(x)))
{
    return f(std::forward<Data>(x));
}

#ifndef JAS_NO_OPTION_SPEC
// template <class Data, class Func, typename std::enable_if<
//                                       Jasnah::IsOption<decltype(std::declval<Func>()(std::forward<Data>(std::declval<Data>())))>::value
//                                       >::type
//                                       >
// auto operator|(Jasnah::Option<Data>&& x, const Func& f)
//     -> decltype(f(std::forward<Data>(*x)))
// {
//     typedef decltype(f(std::forward<Data>(*x))) RetType;

//     if (!x)
//     {
//         if (Jasnah::IsOption<RetType>::value)
//         {
//             return RetType(Jasnah::None);
//         }
//         // else
//         //     throw std::invalid_argument("Option stuff");
//     }


//     return f(std::forward<Data>(*x));
// }

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

// http://stackoverflow.com/a/27885283/3847013
// // primary template.
// template<class T>
// struct function_traits : function_traits<decltype(&T::operator())> {
// };

// // partial specialization for function type
// template<class R, class... Args>
// struct function_traits<R(Args...)> {
//     using result_type = R;
//     using argument_types = std::tuple<Args...>;
// };

// // partial specialization for function pointer
// template<class R, class... Args>
// struct function_traits<R (*)(Args...)> {
//     using result_type = R;
//     using argument_types = std::tuple<Args...>;
// };

// // partial specialization for std::function
// template<class R, class... Args>
// struct function_traits<std::function<R(Args...)>> {
//     using result_type = R;
//     using argument_types = std::tuple<Args...>;
// };

// // partial specialization for pointer-to-member-function (i.e., operator()'s)
// template<class T, class R, class... Args>
// struct function_traits<R (T::*)(Args...)> {
//     using result_type = R;
//     using argument_types = std::tuple<Args...>;
// };

// template<class T, class R, class... Args>
// struct function_traits<R (T::*)(Args...) const> {
//     using result_type = R;
//     using argument_types = std::tuple<Args...>;
// };

// // additional cv-qualifier and ref-qualifier combinations omitted
// // sprinkle with C-style variadics if desired
// template<class T>
// using first_argument_type = typename std::tuple_element<0, typename function_traits<T>::argument_types>::type;

#endif

// Curry operators
#if 0
template<typename Func, typename FnArg>
auto
operator<<=(const Func& f, FnArg&& fnArg)
    -> decltype(f.template LeftCurry<FnArg>(std::forward<FnArg>(fnArg)))
{
    return f.template LeftCurry<FnArg>(std::forward<FnArg>(fnArg));
}

// template<typename Func, typename FnArg>
// auto
// operator >>=(const Func& f, FnArg&& fnArg)
//     -> decltype(f.template RightCurry<FnArg>(std::forward<FnArg>(fnArg)))
// {
//     return f.template RightCurry<FnArg>(std::forward<FnArg>(fnArg));
// }

template<typename Func, typename FnArg>
auto
operator>>=(FnArg&& fnArg, const Func& f)
    -> decltype(f.template RightCurry<FnArg>(std::forward<FnArg>(fnArg)))
{
    return f.template RightCurry<FnArg>(std::forward<FnArg>(fnArg));
}
#else
template<typename Func, typename FnArg>
constexpr auto
operator<<=(const Func& f, FnArg&& fnArg)
    -> decltype(f.template RightCurry<FnArg>(std::forward<FnArg>(fnArg)))
{
    return f.template RightCurry<FnArg>(std::forward<FnArg>(fnArg));
}

// template<typename Func, typename FnArg>
// auto
// operator >>=(const Func& f, FnArg&& fnArg)
//     -> decltype(f.template RightCurry<FnArg>(std::forward<FnArg>(fnArg)))
// {
//     return f.template RightCurry<FnArg>(std::forward<FnArg>(fnArg));
// }

template<typename Func, typename FnArg>
constexpr auto
operator>>=(FnArg&& fnArg, const Func& f)
    -> decltype(f.template LeftCurry<FnArg>(std::forward<FnArg>(fnArg)))
{
    return f.template LeftCurry<FnArg>(std::forward<FnArg>(fnArg));
}
// template <typename Func, typename FnArg>
// constexpr auto
// operator>>(FnArg&& fnArg, const Func& f)
//     -> decltype(f(std::forward<FnArg>(fnArg)))
// {
//     return f(std::forward<FnArg>(fnArg));
// }

// template <typename Func, typename FnArg>
// constexpr auto
// operator<<(const Func& f, FnArg&& fnArg)
//     -> decltype(f(std::forward<FnArg>(fnArg)))
// {
//     return f(std::forward<FnArg>(fnArg));
// }
#endif

#endif
