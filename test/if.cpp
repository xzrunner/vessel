#include <catch/catch.hpp>

#include <vm.h>
#include <debug.h>

TEST_CASE("dangling_else")
{
    ves_str_buf_clear();

    interpret(NULL, R"(
// A dangling else binds to the right-most if.
if (true) if (false) print "bad" else print "good" // expect: good
if (false) if (true) print "bad" else print "bad"
)");
    REQUIRE(std::string(ves_get_str_buf()) == R"(
good
)" + 1);
}

TEST_CASE("else")
{
    ves_str_buf_clear();

    interpret(NULL, R"(
// Evaluate the 'else' expression if the condition is false.
if (true) print "good" else print "bad" // expect: good
if (false) print "bad" else print "good" // expect: good

// Allow block body.
if (false) nil else { print "block" } // expect: block
)");
    REQUIRE(std::string(ves_get_str_buf()) == R"(
good
good
block
)" + 1);
}

TEST_CASE("if")
{
    ves_str_buf_clear();

    interpret(NULL, R"(
// Evaluate the 'then' expression if the condition is true.
if (true) print "good" // expect: good
if (false) print "bad"

// Allow block body.
if (true) { print "block" } // expect: block

// Assignment in if condition.
var a = false
if (a = true) print a // expect: true
)");
    REQUIRE(std::string(ves_get_str_buf()) == R"(
good
block
true
)" + 1);
}

TEST_CASE("truth")
{
    ves_str_buf_clear();

    interpret(NULL, R"(
// False and nil are false.
if (false) print "bad" else print "false" // expect: false
if (nil) print "bad" else print "nil" // expect: nil

// Everything else is true.
if (true) print true // expect: true
if (0) print 0 // expect: 0
if ("") print "empty" // expect: empty
)");
    REQUIRE(std::string(ves_get_str_buf()) == R"(
false
nil
true
0
empty
)" + 1);
}
