#include "utility.h"

#include <catch/catch.hpp>

#include <vessel.h>

TEST_CASE("range")
{
    init_output_buf();

    ves_interpret("test", R"(
System.print(11..22)
)");
    REQUIRE(std::string(get_output_buf()) == R"(
range(11, 22)
)" + 1);
}

TEST_CASE("range_from")
{
    init_output_buf();

    ves_interpret("test", R"(
// Ordered range.
System.print((2..5).from) // expect: 2
System.print((3..3).from) // expect: 3
System.print((0..3).from) // expect: 0
System.print((-5..3).from) // expect: -5
System.print((-5..-2).from) // expect: -5

// Backwards range.
System.print((5..2).from) // expect: 5
System.print((3..0).from) // expect: 3
System.print((3..-5).from) // expect: 3
System.print((-2..-5).from) // expect: -2
)");
    REQUIRE(std::string(get_output_buf()) == R"(
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
    init_output_buf();

    ves_interpret("test", R"(
// Ordered range.
System.print((2..5).to) // expect: 5
System.print((3..3).to) // expect: 3
System.print((0..3).to) // expect: 3
System.print((-5..3).to) // expect: 3
System.print((-5..-2).to) // expect: -2

// Backwards range.
System.print((5..2).to) // expect: 2
System.print((3..0).to) // expect: 0
System.print((3..-5).to) // expect: -5
System.print((-2..-5).to) // expect: -5
)");
    REQUIRE(std::string(get_output_buf()) == R"(
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
    init_output_buf();

    ves_interpret("test", R"(
// Ordered range.
System.print((2..=5).contains(1)) // expect: false
System.print((2..=5).contains(2)) // expect: true
System.print((2..=5).contains(5)) // expect: true
System.print((2..=5).contains(6)) // expect: false

// Backwards range.
System.print((5..=2).contains(1)) // expect: false
System.print((5..=2).contains(2)) // expect: true
System.print((5..=2).contains(5)) // expect: true
System.print((5..=2).contains(6)) // expect: false

// Exclusive ordered range.
System.print((2..5).contains(1)) // expect: false
System.print((2..5).contains(2)) // expect: true
System.print((2..5).contains(5)) // expect: false
System.print((2..5).contains(6)) // expect: false

// Exclusive backwards range.
System.print((5..2).contains(1)) // expect: false
System.print((5..2).contains(2)) // expect: false
System.print((5..2).contains(5)) // expect: true
System.print((5..2).contains(6)) // expect: false
)");
    REQUIRE(std::string(get_output_buf()) == R"(
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