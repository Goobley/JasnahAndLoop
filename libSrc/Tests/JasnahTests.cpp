/* ==========================================================================
   $File: JasnahTests.cpp $
   $Version: 1.0 $
   $Notice: (C) Copyright 2015 Chris Osborne. All Rights Reserved. $
   $License: MIT: http://opensource.org/licenses/MIT $
   ========================================================================== */
#include "../Jasnah.hpp"
#define CATCH_CONFIG_MAIN
#include "../../Tests/catch.hpp"
#include <vector>
#include <algorithm>
#include <numeric>
#include <list>
#include <x86intrin.h>

using std::begin;
using std::end;

auto Add = Jasnah::MakeCurry([](int x, int y)
                             {
                                 return x+y;
                             });
auto Mul = Jasnah::MakeCurry([](int x, int y)
                             {
                                 return x*y;
                             });
auto Div = Jasnah::MakeCurry([](int x, int y)
                             {
                                 return x/y;
                             });
int Identity(int a)
{
    return a;
}

auto Id = Jasnah::MakeCurry(Identity);

template <typename T>
struct DubObj
{
    static T Double(T val) { return 2 * val; }
};

auto Double = Jasnah::MakeCurry(DubObj<int>::Double);

template <typename T>
T Trip(T val)
{
    return 3 * val;
}

// This is essentially equivalent to what JAS_TEMPLATE_FN(Triple, Trip) does
// struct TripImpl
// {
//     template <typename... Args>
//     constexpr auto
//     operator()(Args&&... args) const
//         -> decltype(Trip(std::forward<Args>(args)...))
//     {
//         return Trip(std::forward<Args>(args)...);
//     }
// };
// auto Triple = Jasnah::MakeCurry(TripImpl());
JAS_TEMPLATE_FN(Triple, Trip);

TEST_CASE("Basic Piping")
{
    const int y = 2 | (Add << 2) | (Mul << 5) | (Add << 5);
    REQUIRE(y == 25);

    const int x = 4 | (Div << 2) | (10 >> Mul);
    // 4 / 2 * 10
    REQUIRE(x == 20);

    const int z = 4 | (2 >> Div) | (Mul << 10);
    // 2 / 4 * 10
    REQUIRE(z == 0);

    const int a = (Div << 2 << 2)();
    REQUIRE(a == 1);

    const int b = 1 | Id;
    REQUIRE(b == 1);

    const int c = 2 | Double;
    REQUIRE(c == 4);

    const double d = 3.0 | Triple;
    REQUIRE(d == 9.0);
}


TEST_CASE("Container Processing")
{
    SECTION("vector")
    {
        std::vector<int> v(10000);
        std::iota(begin(v), end(v), 0);
        // {0, 1, 2, 3, 4, 5, 6, 7, 8, 9}
        // const std::vector<int> vStart = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
        // REQUIRE(v == vStart);

        std::size_t start = __rdtsc();
        // auto result = v | Jasnah::Where << [](int x) { return x % 5 == 0; }
        //     | Jasnah::Map << [](int x) { return x*2; };
        for (int i = 0; i < 10000; ++i)
        {
            // auto result = v | (Jasnah::MakeCurry(Jasnah::Where()) << [](int x) { return x % 5 == 0; })
            //     | (Jasnah::MakeCurry(Jasnah::Map()) << [](int x) { return x*2; });
            auto result = v | Jasnah::Where << [](int x) { return x > 5; }
                | Jasnah::Map << [](int x) { return x*2; };
        }
        std::size_t end = __rdtsc();
        // {0, 1, 2, 3, 4, 5, 6, 7, 8, 9}
        // => {6, 7, 8, 9}
        // => {12, 14, 16, 18}

        // const std::vector<int> vEnd = {12, 14, 16, 18};

        // REQUIRE(result == vEnd);

        // Compare timing with naive
#if 1
        CHECK(end - start == 0);
        std::size_t start2 = __rdtsc();
        for (int i = 0; i < 10000; ++i)
        {
            std::vector<int> out;
            out.reserve(v.size());
            for (const auto& x : v)
            {
                if (x > 5)
                    out.push_back(2*x);
            }
            // std::vector<int> out2;
            // out2.reserve(out.size());
            // for (const auto& x : out)
            // {
            //     out2.push_back(2*x);
            // }
        }
        std::size_t end2 = __rdtsc();
        CHECK(end2 - start2 == 0);
#endif
    }

//     SECTION("list")
//     {
//         std::list<int> l(10);
//         std::iota(begin(l), end(l), 0);
//         // {0, 1, 2, 3, 4, 5, 6, 7, 8, 9}
//         const std::list<int> lStart = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
//         REQUIRE(l == lStart);

//         // auto lWhere = Jasnah::WhereFor<std::list<int> >();
//         // auto lMap = Jasnah::MapFor<std::list<int> >();
//         using Jasnah::Piped;
//         using Jasnah::Where;
//         using Jasnah::Map;

//         std::size_t start = __rdtsc();
//         // auto result = l | lWhere([](int x) { return x > 5; }) | lMap([](int x) { return x*2; });
//         auto result = l | Piped(Where()) << ([](int x) { return x > 5; }) | Piped(Map()) << ([](int x) { return x*2; });
//         std::size_t end = __rdtsc();
//         // {0, 1, 2, 3, 4, 5, 6, 7, 8, 9}
//         // => {6, 7, 8, 9}
//         // => {12, 14, 16, 18}

//         const std::list<int> lEnd = {12, 14, 16, 18};

//         REQUIRE(result == lEnd);

//         // Compare timing with naive
// #if 1
//         CHECK(end - start == 0);
//         std::size_t start2 = __rdtsc();
//         std::list<int> out;
//         for (const auto& x : lStart)
//         {
//             if (x > 5)
//                 out.push_back(2*x);
//         }
//         // std::list<int> out2;
//         // for (const auto& x : out)
//         // {
//         //     out2.push_back(2*x);
//         // }
//         std::size_t end2 = __rdtsc();
//         CHECK(end2 - start2 == 0);
// #endif

//     }
}

auto OptAdd = Jasnah::MakeCurry([](Jasnah::Option<int> x, int y)
                                -> Jasnah::Option<int>
                                {
                                    return *x+y;
                                });

auto AddToOpt = Jasnah::MakeCurry([](int x, int y)
                                  -> Jasnah::Option<int>
                                  {
                                      if (x==2)
                                          return Jasnah::None;
                                      return x+y;
                                  });


// TODO(Chris): Can we do something with SFINAE to detect if it's an
// Option<T>, i.e. declare a basic template, then a template template
// and check if it's Option<T>?

TEST_CASE("Option handling")
{
    const Jasnah::Option<int> y = Jasnah::Option<int>(2) | Add << 2;
    REQUIRE(y == 4);
    using Jasnah::Option;

    REQUIRE(Jasnah::IsOption<typename std::decay<Option<int> >::type>::value);
    REQUIRE_FALSE(Jasnah::IsOption<typename std::decay<
                  decltype(Option<int>(3) | (Add << 2))>::type>::value);
    const Option<int> z = Option<int>(2) | (AddToOpt << 2);
    REQUIRE(!z);

    const Jasnah::Option<int> bb = Jasnah::Option<int>(5) | 2 >> AddToOpt;
    REQUIRE(bb == Jasnah::None);

    // Throws for now, want to work out a better abort strategy
    const int aa = Jasnah::Option<int>(Jasnah::None) | 2 >> Add;
    // Not compiling with current implementation as we can't convert None to int
}

struct UnwrapMe
{
    std::vector<int> a;
    int b;
    double c;
    char* d;
};

// http://stackoverflow.com/a/20170989/3847013
#ifndef _MSC_VER
#include <cxxabi.h>
#endif
template <class T>
std::string
TypenameOf()
{
    typedef typename std::remove_reference<T>::type TR;
    std::unique_ptr<char, void(*)(void*)> own
           (
#ifndef _MSC_VER
                abi::__cxa_demangle(typeid(TR).name(), nullptr,
                                           nullptr, nullptr),
#else
                nullptr,
#endif
                std::free
           );
    std::string r = own != nullptr ? own.get() : typeid(TR).name();
    if (std::is_const<TR>::value)
        r += " const";
    if (std::is_volatile<TR>::value)
        r += " volatile";
    if (std::is_lvalue_reference<T>::value)
        r += "&";
    else if (std::is_rvalue_reference<T>::value)
        r += "&&";
    return r;
}

TEST_CASE("Unwrapping")
{
    const char* str = "stuff";
    UnwrapMe uw;
    uw.d = const_cast<char*>(str); // I know this is unsafe it's fine here though

    // auto& a = uw.a;
    // auto& b = uw.b;
    // auto& c = uw.c;
    // auto& d = uw.d;
    JasUnpack(uw, a, b, c, d);

    WARN(TypenameOf<decltype(a)>().c_str());
    WARN(TypenameOf<decltype(b)>().c_str());
    WARN(TypenameOf<decltype(c)>().c_str());
    WARN(TypenameOf<decltype(d)>().c_str());

    d = nullptr;

    CHECK(!uw.d);

    d = const_cast<char*>(str);

    CHECK(uw.d == str);

    b = 10;

    CHECK(uw.b == 10);


}
