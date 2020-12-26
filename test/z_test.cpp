#include <catch/catch.hpp>

#include <vessel.h>

TEST_CASE("precedence")
{
    ves_str_buf_clear();

    interpret(NULL, R"(
// * has higher precedence than +.
print 2 + 3 * 4 // expect: 14

// * has higher precedence than -.
print 20 - 3 * 4 // expect: 8

// / has higher precedence than +.
print 2 + 6 / 3 // expect: 4

// / has higher precedence than -.
print 2 - 6 / 3 // expect: 0

// < has higher precedence than ==.
print false == 2 < 1 // expect: true

// > has higher precedence than ==.
print false == 1 > 2 // expect: true

// <= has higher precedence than ==.
print false == 2 <= 1 // expect: true

// >= has higher precedence than ==.
print false == 1 >= 2 // expect: true

// 1 - 1 is not space-sensitive.
print 1 - 1 // expect: 0
print 1 -1  // expect: 0
print 1- 1  // expect: 0
print 1-1   // expect: 0

// Using () for grouping.
print (2 * (6 - (2 + 2))) // expect: 4
)");
    REQUIRE(std::string(ves_get_str_buf()) == R"(
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

