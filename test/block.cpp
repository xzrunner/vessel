#include <catch/catch.hpp>

#include <vessel.h>

TEST_CASE("empty")
{
    ves_str_buf_clear();

    interpret("test", R"(
{} // By itself.

// In a statement.
if (true) {}
if (false) {} else {}

print "ok" // expect: ok
)");
    REQUIRE(std::string(ves_get_str_buf()) == R"(
ok
)" + 1);
}

TEST_CASE("block-scope")
{
    ves_str_buf_clear();

    interpret("test", R"(
var a = "outer"

{
  var a = "inner"
  print a // expect: inner
}

print a // expect: outer
)");
    REQUIRE(std::string(ves_get_str_buf()) == R"(
inner
outer
)" + 1);
}
