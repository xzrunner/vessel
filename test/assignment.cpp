#include <catch/catch.hpp>

#include <vessel.h>

TEST_CASE("associativity")
{
    ves_str_buf_clear();

    interpret("test", R"(
var a = "a"
var b = "b"
var c = "c"

// Assignment is right-associative.
a = b = c
print a // expect: c
print b // expect: c
print c // expect: c
)");
    REQUIRE(std::string(ves_get_str_buf()) == R"(
c
c
c
)" + 1);
}

TEST_CASE("global")
{
    ves_str_buf_clear();

    interpret("test", R"(
var a = "before"
print a // expect: before

a = "after"
print a // expect: after

print a = "arg" // expect: arg
print a // expect: arg
)");
    REQUIRE(std::string(ves_get_str_buf()) == R"(
before
after
arg
arg
)" + 1);
}

TEST_CASE("local")
{
    ves_str_buf_clear();

    interpret("test", R"(
{
  var a = "before"
  print a // expect: before

  a = "after"
  print a // expect: after

  print a = "arg" // expect: arg
  print a // expect: arg
}
)");
    REQUIRE(std::string(ves_get_str_buf()) == R"(
before
after
arg
arg
)" + 1);
}

TEST_CASE("assign_syntax")
{
    ves_str_buf_clear();

    interpret("test", R"(
// Assignment on RHS of variable.
var a = "before"
var c = a = "var"
print a // expect: var
print c // expect: var
)");
    REQUIRE(std::string(ves_get_str_buf()) == R"(
var
var
)" + 1);
}