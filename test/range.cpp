#include <catch/catch.hpp>

#include <vessel.h>

TEST_CASE("range")
{
    ves_str_buf_clear();

    ves_interpret("test", R"(
print(11..22)
)");
    REQUIRE(std::string(ves_get_str_buf()) == R"(
range(11, 22)
)" + 1);
}

TEST_CASE("range_from")
{
    ves_str_buf_clear();

    ves_interpret("test", R"(
// Ordered range.
print((2..5).from) // expect: 2
print((3..3).from) // expect: 3
print((0..3).from) // expect: 0
print((-5..3).from) // expect: -5
print((-5..-2).from) // expect: -5

// Backwards range.
print((5..2).from) // expect: 5
print((3..0).from) // expect: 3
print((3..-5).from) // expect: 3
print((-2..-5).from) // expect: -2
)");
    REQUIRE(std::string(ves_get_str_buf()) == R"(
2
3
0
-5
-5
5
3
3
-2
)" + 1);
}

TEST_CASE("range_to")
{
    ves_str_buf_clear();

    ves_interpret("test", R"(
// Ordered range.
print((2..5).to) // expect: 5
print((3..3).to) // expect: 3
print((0..3).to) // expect: 3
print((-5..3).to) // expect: 3
print((-5..-2).to) // expect: -2

// Backwards range.
print((5..2).to) // expect: 2
print((3..0).to) // expect: 0
print((3..-5).to) // expect: -5
print((-2..-5).to) // expect: -5
)");
    REQUIRE(std::string(ves_get_str_buf()) == R"(
5
3
3
3
-2
2
0
-5
-5
)" + 1);
}

TEST_CASE("range_contain")
{
    ves_str_buf_clear();

    ves_interpret("test", R"(
// Ordered range.
print((2..=5).contains(1)) // expect: false
print((2..=5).contains(2)) // expect: true
print((2..=5).contains(5)) // expect: true
print((2..=5).contains(6)) // expect: false

// Backwards range.
print((5..=2).contains(1)) // expect: false
print((5..=2).contains(2)) // expect: true
print((5..=2).contains(5)) // expect: true
print((5..=2).contains(6)) // expect: false

// Exclusive ordered range.
print((2..5).contains(1)) // expect: false
print((2..5).contains(2)) // expect: true
print((2..5).contains(5)) // expect: false
print((2..5).contains(6)) // expect: false

// Exclusive backwards range.
print((5..2).contains(1)) // expect: false
print((5..2).contains(2)) // expect: false
print((5..2).contains(5)) // expect: true
print((5..2).contains(6)) // expect: false
)");
    REQUIRE(std::string(ves_get_str_buf()) == R"(
false
true
true
false
false
true
true
false
false
true
false
false
false
false
true
false
)" + 1);
}