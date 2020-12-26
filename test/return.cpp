#include <catch/catch.hpp>

#include <vessel.h>

TEST_CASE("after_else")
{
    ves_str_buf_clear();

    interpret(NULL, R"(
fun f() {
  if (false) "no" else return "ok"
}

print f() // expect: ok
)");
    REQUIRE(std::string(ves_get_str_buf()) == R"(
ok
)" + 1);
}

TEST_CASE("after_if")
{
    ves_str_buf_clear();

    interpret(NULL, R"(
fun f() {
  if (true) return "ok"
}

print f() // expect: ok
)");
    REQUIRE(std::string(ves_get_str_buf()) == R"(
ok
)" + 1);
}

TEST_CASE("after_while")
{
    ves_str_buf_clear();

    interpret(NULL, R"(
fun f() {
  while (true) return "ok"
}

print f() // expect: ok
)");
    REQUIRE(std::string(ves_get_str_buf()) == R"(
ok
)" + 1);
}

TEST_CASE("in_function")
{
    ves_str_buf_clear();

    interpret(NULL, R"(
fun f() {
  return "ok"
  print "bad"
}

print f() // expect: ok
)");
    REQUIRE(std::string(ves_get_str_buf()) == R"(
ok
)" + 1);
}

TEST_CASE("in_method")
{
    ves_str_buf_clear();

    interpret(NULL, R"(
class Foo {
  method() {
    return "ok"
    print "bad"
  }
}

print Foo().method() // expect: ok
)");
    REQUIRE(std::string(ves_get_str_buf()) == R"(
ok
)" + 1);
}

TEST_CASE("return_nil_if_no_value")
{
    ves_str_buf_clear();

    interpret(NULL, R"(
fun f() {
  return
  print "bad"
}

print f() // expect: nil
)");
    REQUIRE(std::string(ves_get_str_buf()) == R"(
nil
)" + 1);
}
