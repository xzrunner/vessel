#include "utility.h"

#include <catch/catch.hpp>

#include <vessel.h>

TEST_CASE("associativity")
{
    init_output_buf();

    ves_interpret("test", R"(
var a = "a"
var b = "b"
var c = "c"

// Assignment is right-associative.
a = b = c
System.print(a) // expect: c
System.print(b) // expect: c
System.print(c) // expect: c
)");
    REQUIRE(std::string(get_output_buf()) == R"(
c
c
c
)" + 1);
}

TEST_CASE("global")
{
    init_output_buf();

    ves_interpret("test", R"(
var a = "before"
System.print(a) // expect: before

a = "after"
System.print(a) // expect: after

System.print(a = "arg") // expect: arg
System.print(a) // expect: arg
)");
    REQUIRE(std::string(get_output_buf()) == R"(
before
after
arg
arg
)" + 1);
}

TEST_CASE("local")
{
    init_output_buf();

    ves_interpret("test", R"(
{
  var a = "before"
  System.print(a) // expect: before

  a = "after"
  System.print(a) // expect: after

  System.print(a = "arg") // expect: arg
  System.print(a) // expect: arg
}
)");
    REQUIRE(std::string(get_output_buf()) == R"(
before
after
arg
arg
)" + 1);
}

TEST_CASE("assign_syntax")
{
    init_output_buf();

    ves_interpret("test", R"(
// Assignment on RHS of variable.
var a = "before"
var c = a = "var"
System.print(a) // expect: var
System.print(c) // expect: var
)");
    REQUIRE(std::string(get_output_buf()) == R"(
var
var
)" + 1);
}