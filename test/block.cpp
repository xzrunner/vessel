#include "utility.h"

#include <catch/catch.hpp>

#include <vessel.h>

TEST_CASE("empty")
{
    init_output_buf();

    ves_interpret("test", R"(
{} // By itself.

// In a statement.
if (true) {}
if (false) {} else {}

System.print("ok") // expect: ok
)");
    REQUIRE(std::string(get_output_buf()) == R"(
ok
)" + 1);
}

TEST_CASE("block-scope")
{
    init_output_buf();

    ves_interpret("test", R"(
var a = "outer"

{
  var a = "inner"
  System.print(a) // expect: inner
}

System.print(a) // expect: outer
)");
    REQUIRE(std::string(get_output_buf()) == R"(
inner
outer
)" + 1);
}
