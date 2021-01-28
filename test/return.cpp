#include "utility.h"

#include <catch/catch.hpp>

#include <vessel.h>

TEST_CASE("after_else")
{
    init_output_buf();

    ves_interpret("test", R"(
fun f() {
  if (false) "no" else return "ok"
}

System.print(f()) // expect: ok
)");
    REQUIRE(std::string(get_output_buf()) == R"(
ok
)" + 1);
}

TEST_CASE("after_if")
{
    init_output_buf();

    ves_interpret("test", R"(
fun f() {
  if (true) return "ok"
}

System.print(f()) // expect: ok
)");
    REQUIRE(std::string(get_output_buf()) == R"(
ok
)" + 1);
}

TEST_CASE("after_while")
{
    init_output_buf();

    ves_interpret("test", R"(
fun f() {
  while (true) return "ok"
}

System.print(f()) // expect: ok
)");
    REQUIRE(std::string(get_output_buf()) == R"(
ok
)" + 1);
}

TEST_CASE("in_function")
{
    init_output_buf();

    ves_interpret("test", R"(
fun f() {
  return "ok"
  System.print("bad")
}

System.print(f()) // expect: ok
)");
    REQUIRE(std::string(get_output_buf()) == R"(
ok
)" + 1);
}

TEST_CASE("in_method")
{
    init_output_buf();

    ves_interpret("test", R"(
class Foo {
  method() {
    return "ok"
    System.print("bad")
  }
}

System.print(Foo().method()) // expect: ok
)");
    REQUIRE(std::string(get_output_buf()) == R"(
ok
)" + 1);
}

TEST_CASE("return_nil_if_no_value")
{
    init_output_buf();

    ves_interpret("test", R"(
fun f() {
  return
  System.print("bad")
}

System.print(f()) // expect: nil
)");
    REQUIRE(std::string(get_output_buf()) == R"(
nil
)" + 1);
}