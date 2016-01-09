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


namespace Jasnah
{
    // TODO(Chris): Check polution
    namespace JasPH = std::placeholders;

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

    template <class T>
    using InternalType = typename T::value_type;

// Extension methods for container processing
// TODO(Chris): These use push_back for now, fix
// TODO(Chris): Should probably reserve or something
    template <typename T>
    T WhereInContainer(const T& cont, std::function<bool(InternalType<T>)> Property)
    {
        T result;
        for (const auto& x : cont)
        {
            if (Property(x))
            {
                result.push_back(x);
            }
        }
        return result;
    }

    template <class T>
    T MapToContainer(const T& cont, std::function<InternalType<T>
                     (InternalType<T>)> Func)
    {
        T result;
        for (const auto& x : cont)
        {
            result.push_back(Func(x));
        }
        return result;
    }

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

    //NOTE:: Export funcs, make more prominent
    template <class T>
    constexpr auto
    WhereFor()
        -> decltype(MakeFnPipeableTwoArgs(WhereInContainer<T>))
    {
        return MakeFnPipeableTwoArgs(WhereInContainer<T>);
    }

    template <class T>
    constexpr auto
    MapFor()
        -> decltype(MakeFnPipeableTwoArgs(MapToContainer<T>))
    {
        return MakeFnPipeableTwoArgs(MapToContainer<T>);
    }

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

#endif
