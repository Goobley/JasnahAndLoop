// -*- c++ -*-
#if !defined(ERROROR_H)
/* ==========================================================================
   $File: ErrorOr.hpp $
   $Version: 1.0 $
   $Notice: (C) Copyright 2015 Chris Osborne. All Rights Reserved. $
   $License: MIT: http://opensource.org/licenses/MIT $
   ========================================================================== */

#define ERROROR_H
#include <type_traits>
#include <functional>
#include <cassert>

// NOTE(Chris): Just redefine default macro for now
#define Assert(x) assert(x)

namespace Lethani
{
    struct InPlace {};
    constexpr InPlace ConstructInPlace{};

    struct EmptyOption{};
    constexpr EmptyOption None{};

    template <class T>
    class Option
    {
    private:
        template <bool = std::is_trivially_destructible<T>::value>
        struct OptionStorage
        {
            union
            {
                char isNull;
                T value;
            };
            bool some = false;

            ~OptionStorage()
            {
                if (some)
                    value.~T();
            }

            constexpr OptionStorage()
                : isNull('\0')
            {}

            // TODO(Chris): Create our own new etc
            OptionStorage(const OptionStorage& other)
                : some(other.some)
            {
                if (some)
                    new(&this->value) T(other.value);
            }

            OptionStorage(OptionStorage&& other)
                : some(other.some)
            {
                if (some)
                    new(&this->value) T(std::move(other.value));
            }

            constexpr OptionStorage(const T& val)
                : value(val),
                  some(true)
            {}

            constexpr OptionStorage(T&& val)
                : value(std::move(val)),
                  some(true)
            {}

            template <class... Args>
            constexpr explicit
            OptionStorage(InPlace, Args&&... args)
                : value(std::forward<Args>(args)...),
                  some(true)
            {}
        };

        OptionStorage<> storage_;

    public:
        // Standard compliance
        typedef T value_type;

        static_assert(!std::is_reference<T>::value, "Option<T> cannot be instantiated with a reference type");
        static_assert(std::is_object<T>::value, "Option<T> must be instantiated with an object type (i.e. must have storage)");

        constexpr Option()
            : storage_()
        {}
        constexpr Option(EmptyOption)
            : storage_()
        {}
        constexpr Option(const T& val)
            : storage_(val)
        {}
        constexpr Option(T&& val)
            : storage_(std::move(val))
        {}

        Option(const Option&) = default;
        Option(Option&&) = default;
        ~Option() = default;

        // Fail at compile time if the object cannot be constructed as specified
        template <class... Args,
                  class = typename std::enable_if<
                      std::is_constructible<T, Args&&...>::value>::type
                  >
        constexpr explicit
        Option(InPlace, Args&&... args)
            : storage_(ConstructInPlace, std::forward<Args>(args)...)
        {}

        // Fail at compile time if the object cannot be constructed as specified
        template <class U, class... Args,
                  class = typename std::enable_if<
                      std::is_constructible<T, std::initializer_list<U>&, Args&&...>::value>::type
                  >
        constexpr explicit
        Option(InPlace, std::initializer_list<U> ilist, Args&&... args)
            : storage_(ConstructInPlace, ilist, std::forward<Args>(args)...)
        {}

        Option&
        operator=(EmptyOption)
        {
            if (storage_.some)
            {
                storage_.value.~T();
                storage_.some = false;
            }
            return *this;
        }

        Option&
        operator=(const Option& other)
        {
            if (storage_.some && other.storage_.some)
            {
                storage_.value = other.storage_.value;
            }
            else if (storage_.some)
            {
                storage_.value.~T();
                storage_.some = false;
            }
            else if (other.storage_.some)
            {
                // TODO(Chris): Fix new here
                new(&(storage_.value)) T(other.storage_.value);
                storage_.some = true;
            }
            return *this;
        }

        Option&
        operator=(Option&& other)
        {
            if (storage_.some && other.storage_.some)
            {
                storage_.value = std::move(other.storage_.value);
            }
            else if (storage_.some)
            {
                storage_.value.~T();
                storage_.some = false;
            }
            else if (other.storage_.some)
            {
                // TODO(Chris): Fix new here
                new(&(storage_.value)) T(std::move(other.storage_.value));
                storage_.some = true;
            }
            return *this;
        }

        // Just following the spec for this one, makes sense though -
        // only enable if U can decay to a T (true if U is a T)
        template <class U,
                  class = typename std::enable_if<std::is_same<typename std::decay<U>::type, T>::value>::type
                  >
        Option&
        operator=(U&& val)
        {
            if (storage_.some)
                storage_.value = std::forward<U>(val);
            else
            {
                // TODO(Chris): Another new here
                new(&(storage_.value)) T(std::forward<U>(val));
                storage_.some = true;
            }
            return *this;
        }

        template <class... Args,
                  class = typename std::enable_if<
                      std::is_constructible<T, Args&&...>::value>::type
                  >
        void
        Emplace(Args&&... args)
        {
            *this = None;
            // TODO(Chris): Another new to be replaced
            new(&(storage_.value)) T(std::forward<Args>(args)...);
            storage_.some = true;
        }

        template <class U, class... Args,
                  class = typename std::enable_if<
                      std::is_constructible<T, std::initializer_list<U>&, Args&&...>::value>::type
                  >
        void
        Emplace(std::initializer_list<U> ilist, Args&&... args)
        {
            *this = None;
            // TODO(Chris): Yet another new...
            new(&(storage_.value)) T(ilist, std::forward<Args>(args)...);
            storage_.some = true;
        }

        constexpr const T*
        operator->() const
        {
            Assert(storage_.some && "Option dereferenced (->) for None");
            return &(storage_.value);
        }

        T*
        operator->()
        {
            Assert(storage_.some && "Option dereferenced (->) for None");
            return &(storage_.value);
        }

        constexpr const T&
        operator*() const
        {
            Assert(storage_.some && "Option dereferenced (*) for None");
            return storage_.value;
        }

        T&
        operator*()
        {
            Assert(storage_.some && "Option dereferenced (*) for None");
            return storage_.value;
        }

        // NOTE(Chris): Do we want an explicit Value() method that is unsafe?

        constexpr explicit
        operator bool() const
        {
            return storage_.some;
        }

        inline constexpr
        bool
        IsNone() const
        {
            return !(storage_.some);
        }

        template <class U>
        constexpr T
        ValueOr(U&& defaultVal) const& // to be used when this object is an lvalue
        {
            static_assert(std::is_copy_constructible<T>::value,
                          "T must be copy constructible to use Option<T>::ValueOr");
            static_assert(std::is_convertible<U, T>::value,
                          "U must be convertible to T to use Option<T>::ValueOr");

            return (storage_.some) ? storage_.value :
                static_cast<T>(std::forward<U>(defaultVal));
        }

        template <class U>
        T
        ValueOr(U&& defaultVal) && // Used when this object is an rvalue
        {
            static_assert(std::is_move_constructible<T>::value,
                          "T must be move constructible to use Option<T>::ValueOr");
            static_assert(std::is_convertible<U, T>::value,
                          "U must be convertible to T to use Option<T>::ValueOr");
            return (storage_.some) ? std::move(storage_.value) :
                static_cast<T>(std::forward<U>(defaultVal));
        }

        // TODO(Chris): swap override?

    };

    // All the comparison operators. Oh the joys of C++11 constexpr
    template <class T>
    inline constexpr
    bool
    operator==(const Option<T>& lhs, const Option<T>& rhs)
    {
        return (static_cast<bool>(lhs) != static_cast<bool>(rhs))
            ? false
            : ((!static_cast<bool>(rhs))
               ? true
               : (*lhs == *rhs));
    }

    template <class T>
    inline constexpr
    bool
    operator!=(const Option<T>& lhs, const Option<T>& rhs)
    {
        return !(lhs == rhs);
    }

    template <class T>
    inline constexpr
    bool
    operator<(const Option<T>& lhs, const Option<T>& rhs)
    {
        return (!static_cast<bool>(rhs))
            ? false
            : (!(static_cast<bool>(lhs))
               ? true
               : (*lhs < *rhs));
    }

    template <class T>
    inline constexpr
    bool
    operator<=(const Option<T>& lhs, const Option<T>& rhs)
    {
        return !(rhs < lhs);
    }

    template <class T>
    inline constexpr
    bool
    operator>(const Option<T>& lhs, const Option<T>& rhs)
    {
        return rhs < lhs;
    }

    template <class T>
    inline constexpr
    bool
    operator>=(const Option<T>& lhs, const Option<T>& rhs)
    {
        return !(lhs < rhs);
    }

    // Compare with EmptyOption for sorting
    template <class T>
    inline constexpr
    bool
    operator==(const Option<T>& x, EmptyOption)
    {
        return !(static_cast<bool>(x));
    }

    template <class T>
    inline constexpr
    bool
    operator==(EmptyOption, const Option<T>& x)
    {
        return !(static_cast<bool>(x));
    }

    template <class T>
    inline constexpr
    bool
    operator!=(const Option<T>& x, EmptyOption)
    {
        return static_cast<bool>(x);
    }

    template <class T>
    inline constexpr
    bool
    operator!=(EmptyOption, const Option<T>& x)
    {
        return static_cast<bool>(x);
    }

    template <class T>
    inline constexpr
    bool
    operator<(const Option<T>& x, EmptyOption)
    {
        return false;
    }

    template <class T>
    inline constexpr
    bool
    operator<(EmptyOption, const Option<T>& x)
    {
        return static_cast<bool>(x);
    }

    template <class T>
    inline constexpr
    bool
    operator<=(const Option<T>& x, EmptyOption)
    {
        return !(static_cast<bool>(x));
    }

    template <class T>
    inline constexpr
    bool
    operator<=(EmptyOption, const Option<T>& x)
    {
        return true;
    }

    template <class T>
    inline constexpr
    bool
    operator>(const Option<T>& x, EmptyOption)
    {
        return static_cast<bool>(x);
    }

    template <class T>
    inline constexpr
    bool
    operator>(EmptyOption, const Option<T>& x)
    {
        return false;
    }

    template <class T>
    inline constexpr
    bool
    operator>=(const Option<T>& x, EmptyOption)
    {
        return true;
    }

    template <class T>
    inline constexpr
    bool
    operator>=(EmptyOption, const Option<T>& x)
    {
        return !(static_cast<bool>(x));
    }

    // Compare Option<T> with a T
    template <class T>
    inline constexpr
    bool
    operator==(const Option<T>& x, const T& val)
    {
        return (static_cast<bool>(x))
            ? (*x == val)
            : false;
    }

    template <class T>
    inline constexpr
    bool
    operator==(const T& val, const Option<T>& x)
    {
        return (static_cast<bool>(x))
            ? (val == *x)
            : false;
    }

    template <class T>
    inline constexpr
    bool
    operator!=(const Option<T>& x, const T& val)
    {
        return (static_cast<bool>(x))
            ? !(*x == val)
            : true;
    }

    template <class T>
    inline constexpr
    bool
    operator!=(const T& val, const Option<T>& x)
    {
        return (static_cast<bool>(x))
            ? !(val == *x)
            : true;
    }

    template <class T>
    inline constexpr
    bool
    operator<(const Option<T>& x, const T& val)
    {
        return (static_cast<bool>(x))
            ? (*x < val)
            : true;
    }

    template <class T>
    inline constexpr
    bool
    operator<(const T& val, const Option<T>& x)
    {
        return (static_cast<bool>(x))
            ? (val < *x)
            : false;
    }

    template <class T>
    inline constexpr
    bool
    operator>(const Option<T>& x, const T& val)
    {
        return (static_cast<bool>(x))
            ? (val < *x)
            : false;
    }

    template <class T>
    inline constexpr
    bool
    operator>(const T& val, const Option<T>& x)
    {
        return (static_cast<bool>(x))
            ? (*x < val)
            : true;
    }

    template <class T>
    inline constexpr
    bool
    operator<=(const Option<T>& x, const T& val)
    {
        return !(x > val);
    }

    template <class T>
    inline constexpr
    bool
    operator<=(const T& val, const Option<T>& x)
    {
        return !(val > x);
    }

    template <class T>
    inline constexpr
    bool
    operator>=(const Option<T>& x, const T& val)
    {
        return !(x < val);
    }

    template <class T>
    inline constexpr
    bool
    operator>=(const T& val, const Option<T>& x)
    {
        return !(val < x);
    }


    // Helper function
    template <class T>
    inline constexpr
    Option<typename std::decay<T>::type>
    MakeOption(T&& val)
    {
        return Option<typename std::decay<T>::type>(std::forward<T>(val));
    }
}
#undef Assert
#endif
