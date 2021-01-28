#include "utility.h"

#include <catch/catch.hpp>

#include <vessel.h>

TEST_CASE("dangling_else")
{
    init_output_buf();

    ves_interpret("test", R"(
// A dangling else binds to the right-most if.
if (true) if (false) System.print("bad") else System.print("good") // expect: good
if (false) if (true) System.print("bad") else System.print("bad")
)");
    REQUIRE(std::string(get_output_buf()) == R"(
good
)" + 1);
}

TEST_CASE("else")
{
    init_output_buf();

    ves_interpret("test", R"(
// Evaluate the 'else' expression if the condition is false.
if (true) System.print("good") else System.print("bad") // expect: good
if (false) System.print("bad") else System.print("good") // expect: good

// Allow block body.
if (false) nil else { System.print("block") } // expect: block
)");
    REQUIRE(std::string(get_output_buf()) == R"(
good
good
block
)" + 1);
}

TEST_CASE("if")
{
    init_output_buf();

    ves_interpret("test", R"(
// Evaluate the 'then' expression if the condition is true.
if (true) System.print("good") // expect: good
if (false) System.print("bad")

// Allow block body.
if (true) { System.print("block") } // expect: block

// Assignment in if condition.
var a = false
if (a = true) System.print(a) // expect: true
)");
    REQUIRE(std::string(get_output_buf()) == R"(
good
block
true
)" + 1);
}

TEST_CASE("truth")
{
    init_output_buf();

    ves_interpret("test", R"(
// False and nil are false.
if (false) System.print("bad") else System.print("false") // expect: false
if (nil) System.print("bad") else System.print("nil") // expect: nil

// Everything else is true.
if (true) System.print(true) // expect: true
if (0) System.print(0) // expect: 0
if ("") System.print("empty") // expect: empty
)");
    REQUIRE(std::string(get_output_buf()) == R"(
false
nil
true
0
empty
)" + 1);
}