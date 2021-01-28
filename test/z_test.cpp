#include "utility.h"

#include <catch/catch.hpp>

#include <vessel.h>

TEST_CASE("precedence")
{
    init_output_buf();

    ves_interpret("test", R"(
// * has higher precedence than +.
System.print(2 + 3 * 4) // expect: 14

// * has higher precedence than -.
System.print(20 - 3 * 4) // expect: 8

// / has higher precedence than +.
System.print(2 + 6 / 3) // expect: 4

// / has higher precedence than -.
System.print(2 - 6 / 3) // expect: 0

// < has higher precedence than ==.
System.print(false == 2 < 1) // expect: true

// > has higher precedence than ==.
System.print(false == 1 > 2) // expect: true

// <= has higher precedence than ==.
System.print(false == 2 <= 1) // expect: true

// >= has higher precedence than ==.
System.print(false == 1 >= 2) // expect: true

// 1 - 1 is not space-sensitive.
System.print(1 - 1) // expect: 0
System.print(1 -1)  // expect: 0
System.print(1- 1)  // expect: 0
System.print(1-1)   // expect: 0

// Using () for grouping.
System.print((2 * (6 - (2 + 2)))) // expect: 4
)");
    REQUIRE(std::string(get_output_buf()) == R"(
14
8
4
0
true
true
true
true
0
0
0
0
4
)" + 1);
}