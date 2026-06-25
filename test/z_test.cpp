#include "utility.h"

#include <catch2/catch_test_macros.hpp>

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

TEST_CASE("multiline line continuation")
{
    init_output_buf();

    ves_interpret("test", R"(
class Foo {
    static bar(x, y, z) {
        return x + y + z
    }
}

// call arguments spanning lines (newline after a comma / open paren)
System.print(Foo.bar(1,
    2,
    3)) // expect: 6

// array literal spanning lines
var a = [10,
    20,
    30]
System.print(a[0] + a[1] + a[2]) // expect: 60

// binary expression spanning lines (newline after an operator)
System.print(100 +
    200) // expect: 300
)");
    REQUIRE(std::string(get_output_buf()) == R"(
6
60
300
)" + 1);
}