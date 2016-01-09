/* ==========================================================================
   $File: OptionTests.cpp $
   $Version: 1.0 $
   $Notice: (C) Copyright 2016 Chris Osborne. All Rights Reserved. $
   $License: MIT: http://opensource.org/licenses/MIT $
   ========================================================================== */

#include "../Option.hpp"
#define CATCH_CONFIG_MAIN
#include "../../Tests/catch.hpp"
#include <vector>


using Lethani::Option;

struct TestOpts { int a, b, c; };
inline constexpr
bool
operator==(const TestOpts& rhs, const TestOpts& lhs)
{
    return (rhs.a == lhs.a)
        && (rhs.b == lhs.b)
        && (rhs.c == lhs.c);
}

inline constexpr
bool
operator!=(const TestOpts& rhs, const TestOpts& lhs)
{
    return !(rhs == lhs);
}

struct NoCopy
{
    int a, b, c;
    NoCopy(const NoCopy&) = delete;
    NoCopy(NoCopy&&) = default;
    NoCopy& operator=(const NoCopy&) = delete;
    NoCopy& operator=(NoCopy&&) = default;
};
inline constexpr
bool
operator==(const NoCopy& rhs, const NoCopy& lhs)
{
    return (rhs.a == lhs.a)
        && (rhs.b == lhs.b)
        && (rhs.c == lhs.c);
}

inline constexpr
bool
operator!=(const NoCopy& rhs, const NoCopy& lhs)
{
    return !(rhs == lhs);
}

struct NonTrivial
{
    std::size_t value;
    int* modify;
    NonTrivial(std::size_t val, int* mod)
        : value(val),
          modify(mod)
    {}

    ~NonTrivial()
    {
        (*modify) += value;
    }
    NonTrivial(const NonTrivial&) = default;
    NonTrivial& operator=(const NonTrivial&) = default;
};
constexpr inline bool
operator==(const NonTrivial& lhs, const NonTrivial& rhs)
{
    return lhs.value == rhs.value;
}

struct VarAndFn
{
    int a;
    VarAndFn(int val) : a(val) {}
    void SetA(int newVal) { a = newVal; }
};

TEST_CASE("Creating Option<T> for simple objects", "[Option]")
{
    SECTION("Option<T> for int")
    {
        Option<int> o(5);

        REQUIRE(o == 5);
        REQUIRE(o > 4);
        REQUIRE(o != Lethani::None);
    }

    SECTION("Empty Option<T>")
    {
        TestOpts a{1, 2, 3};
        TestOpts b{2, 3, 4};
        Option<TestOpts> o(Lethani::None);

        REQUIRE(o.IsNone() == true);
        REQUIRE(o.ValueOr(a) == a);
        REQUIRE(o.ValueOr(a) != b);
    }

    SECTION("Option<PODType>")
    {
        TestOpts a{1, 2, 3};
        TestOpts b{2, 3, 4};
        Option<TestOpts> o1;
        Option<TestOpts> o2(b);

        REQUIRE(o1.IsNone() == true);
        REQUIRE(o2.IsNone() == false);

        o1 = o2;
        REQUIRE(o1.IsNone() == false);
        REQUIRE(o1 == o2);
        REQUIRE(o1 == b);

        o1 = a;
        REQUIRE(o1 == a);
        REQUIRE(o1 != o2);
    }

    SECTION("Non-copyable POD Option<T>")
    {
        using std::move;
        NoCopy a{1, 2, 3};
        NoCopy b{2, 3, 4};
        Option<NoCopy> o1;
        Option<NoCopy> o2(move(b));

        REQUIRE(o1.IsNone() == true);
        REQUIRE(o2.IsNone() == false);

        o1 = move(o2);
        REQUIRE(o1.IsNone() == false);
        REQUIRE(o1 == o2);
        REQUIRE(o1 == b);

        o1 = move(a);
        REQUIRE(o1 == a);
        REQUIRE(o1 != o2);
    }

    SECTION("Non trivial constructor & destructor")
    {
        int val = 4;
        {
            Option<NonTrivial> o(NonTrivial(1, &val));
        }
        // 2 dtors called
        REQUIRE(val == 6);

        {
            Option<NonTrivial> o(Lethani::ConstructInPlace, 1, &val);
        }
        // 1 dtor called
        REQUIRE(val == 7);

        {
            Option<NonTrivial> o = std::move(NonTrivial(1, &val));
        }
        // 2 dtors called
        REQUIRE(val == 9);

        {
            Option<NonTrivial> o(Lethani::ConstructInPlace, 1, &val);
            o.Emplace(1, &val);
        }
        // 2 dtors called
        REQUIRE(val == 11);
    }

    SECTION("Initializer list")
    {
        using std::vector;
        vector<int> v = {1, 2, 3};
        vector<int> v2 = {2, 3, 4};
        Option<vector<int>> o(Lethani::ConstructInPlace, {1, 2, 3});

        REQUIRE(o.IsNone() == false);
        REQUIRE(o.ValueOr(v2) == v);

        o.Emplace({2, 3, 4});
        REQUIRE(o == v2);
    }

    SECTION("Operators")
    {
        VarAndFn vfn(0);
        Option<VarAndFn> o(vfn);

        o->SetA(1);
        REQUIRE(o.ValueOr(vfn).a == 1);
        REQUIRE(o->a == 1);

        (*o).SetA(2);
        REQUIRE(o.ValueOr(vfn).a == 2);
        REQUIRE((*o).a == 2);

        // Uncomment this for an abort to throw if we try to
        // dereference a None Option<T>
        // o = Lethani::None;
        // CHECK_FALSE(o->a == 2);
    }

    SECTION("MakeOption")
    {
        using Lethani::MakeOption;
        int val = 0;
        {
            auto opt(MakeOption(NonTrivial(1, &val)));
        }
        // 2 dtors
        CHECK(val == 2);

        {
            Option<NonTrivial> opt = MakeOption(NonTrivial(1, &val));
        }
        // 2 dtors
        CHECK(val == 4);
    }

}

template <class T>
struct DecisiveObject
{
    int a;
    DecisiveObject() : a(1)
    {}
};

template <typename T>
struct DecisiveObject<Option<T> >
{
    int a;
    DecisiveObject() : a(2)
    {}
};

class error_code
{
    int __val_;
    const std::error_category* __cat_;
public:
    error_code() _NOEXCEPT : __val_(0), __cat_(&std::system_category()) {}

    error_code(int __val, const std::error_category& __cat) _NOEXCEPT
        : __val_(__val), __cat_(&__cat) {}

    template <class _Ep>
        _LIBCPP_ALWAYS_INLINE
        error_code(_Ep __e,
                   typename std::enable_if<std::is_error_code_enum<_Ep>::value>::type* = 0
                                                                     ) _NOEXCEPT
            {*this = make_error_code(__e);}

    _LIBCPP_ALWAYS_INLINE
    void assign(int __val, const std::error_category& __cat) _NOEXCEPT
    {
        __val_ = __val;
        __cat_ = &__cat;
    }

    template <class _Ep>
        _LIBCPP_ALWAYS_INLINE
    typename std::enable_if
        <
        std::is_error_code_enum<_Ep>::value,
            error_code&
        >::type
        operator=(_Ep __e) _NOEXCEPT
            {*this = make_error_code(__e); return *this;}

    _LIBCPP_ALWAYS_INLINE
    void clear() _NOEXCEPT
    {
        __val_ = 0;
        __cat_ = &std::system_category();
    }

    _LIBCPP_ALWAYS_INLINE
    int value() const _NOEXCEPT {return __val_;}

    _LIBCPP_ALWAYS_INLINE
    const std::error_category& category() const _NOEXCEPT {return *__cat_;}

    _LIBCPP_ALWAYS_INLINE
    std::error_condition default_error_condition() const _NOEXCEPT
        {return __cat_->default_error_condition(__val_);}

    std::string message() const;

    _LIBCPP_ALWAYS_INLINE
        _LIBCPP_EXPLICIT
        operator bool() const _NOEXCEPT {return __val_ != 0;}
};

TEST_CASE("Separating Option<T> from other objects", "[Option]")
{
    SECTION("Non-Option")
    {
        DecisiveObject<int> do1;
        DecisiveObject<std::vector<int> > do2;

        REQUIRE(do1.a == 1);
        REQUIRE(do2.a == 1);
    }

    SECTION("Option")
    {
        DecisiveObject<Option<int> > do1;
        DecisiveObject<Option<std::vector<int> > > do2;

        REQUIRE(do1.a == 2);
        REQUIRE(do2.a == 2);
    }

    // SECTION("Size")
    // {
    //     CHECK(sizeof(error_code) == 16);
    //     CHECK(sizeof(std::error_code) == 4);
    //     CHECK(sizeof(Option<char>) == 2);
    // }
}
