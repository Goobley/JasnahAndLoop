/* ==========================================================================
   $File: JasnahTests.cpp $
   $Version: 1.0 $
   $Notice: (C) Copyright 2015 Chris Osborne. All Rights Reserved. $
   $License: MIT: http://opensource.org/licenses/MIT $
   ========================================================================== */
#include "../Jasnah.hpp"
#include "../Option.hpp"
#define CATCH_CONFIG_MAIN
#include "../../Tests/catch.hpp"
#include <vector>
#include <algorithm>
#include <numeric>
#include <list>
#include <x86intrin.h>

using std::begin;
using std::end;

auto Add = Jasnah::Piped([](int x, int y)
                         {
                             return x+y;
                         });
auto Mul = Jasnah::Piped([](int x, int y)
                         {
                             return x*y;
                         });
auto Div = Jasnah::Piped([](int x, int y)
                         {
                             return x/y;
                         });
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

        // auto vWhere = Jasnah::WhereFor<std::vector<int> >();
        // auto vMap = Jasnah::MapFor<std::vector<int> >();

        std::size_t start = __rdtsc();
        // Update Map/Where design
        // UnwrapTupleIntoFn seems the slow point atm due to memory movement/copying
        for (int i = 0; i < 10000; ++i)
        {
            auto result = v | (Jasnah::Piped(Jasnah::Where()) << [](int x) { return x % 5 == 0; })
                | (Jasnah::Piped(Jasnah::Map()) << [](int x) { return x*2; });
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
                if (x % 5 == 0)
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

auto OptAdd = Jasnah::Piped([](Jasnah::Option<int> x, int y)
                         -> Jasnah::Option<int>
                 {
                     return *x+y;
                 });

auto AddToOpt = Jasnah::Piped([](int x, int y)
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
    // const Jasnah::Option<int> y = Jasnah::Option<int>(2) | OptAdd(2);
    // REQUIRE(y == 4);
    using Jasnah::Option;

    // REQUIRE(Jasnah::IsOption<typename std::decay<Option<int> >::type>::value);
    // REQUIRE_FALSE(Jasnah::IsOption<typename std::decay<decltype(Option<int>(3) | (Add << 2))>::type>::value);
    const Option<int> z = Option<int>(3) | (AddToOpt << 2);
    REQUIRE(*z == 5);

    // Jasnah::Option<int> a(5);
    // int b = 5;

    // bool same1 = Jasnah::IsOption<decltype(a)>::value;
    // bool same2 = Jasnah::IsOption<decltype(b)>::value;


    // REQUIRE(same1);
    // REQUIRE_FALSE(same2);

    // const Jasnah::Option<int> bb = Jasnah::Option<int>(2) | AddToOpt(2);
    // REQUIRE(bb == Jasnah::None);



    // Throws for now, want to work out a better abort strategy
    // const int aa = Jasnah::Option<int>(Jasnah::None) | add(2);
    // Not compiling with current implementation as we can't convert None to int
}
