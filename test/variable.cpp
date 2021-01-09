#include <catch/catch.hpp>

#include <vessel.h>

TEST_CASE("early_bound")
{
    vessel_str_buf_clear();

    vessel_interpret("test", R"(
var a = "outer"
{
  fun foo() {
    print a
  }

  foo() // expect: outer
  var a = "inner"
  foo() // expect: outer
}
)");
    REQUIRE(std::string(vessel_get_str_buf()) == R"(
outer
outer
)" + 1);
}

TEST_CASE("in_middle_of_block")
{
    vessel_str_buf_clear();

    vessel_interpret("test", R"(
{
  var a = "a"
  print a // expect: a
  var b = a + " b"
  print b // expect: a b
  var c = a + " c"
  print c // expect: a c
  var d = b + " d"
  print d // expect: a b d
}
)");
    REQUIRE(std::string(vessel_get_str_buf()) == R"(
a
a b
a c
a b d
)" + 1);
}

TEST_CASE("in_nested_block")
{
    vessel_str_buf_clear();

    vessel_interpret("test", R"(
{
  var a = "outer"
  {
    print a // expect: outer
  }
}
)");
    REQUIRE(std::string(vessel_get_str_buf()) == R"(
outer
)" + 1);
}

TEST_CASE("local_from_method")
{
    vessel_str_buf_clear();

    vessel_interpret("test", R"(
var foo = "variable"

class Foo {
  method() {
    print foo
  }
}

Foo().method() // expect: variable
)");
    REQUIRE(std::string(vessel_get_str_buf()) == R"(
variable
)" + 1);
}

TEST_CASE("redeclare_global")
{
    vessel_str_buf_clear();

    vessel_interpret("test", R"(
var a = "1"
var a
print a // expect: nil
)");
    REQUIRE(std::string(vessel_get_str_buf()) == R"(
nil
)" + 1);
}

TEST_CASE("redefine_global")
{
    vessel_str_buf_clear();

    vessel_interpret("test", R"(
var a = "1"
var a = "2"
print a // expect: 2
)");
    REQUIRE(std::string(vessel_get_str_buf()) == R"(
2
)" + 1);
}

TEST_CASE("scope_reuse_in_different_blocks")
{
    vessel_str_buf_clear();

    vessel_interpret("test", R"(
{
  var a = "first"
  print a // expect: first
}

{
  var a = "second"
  print a // expect: second
}
)");
    REQUIRE(std::string(vessel_get_str_buf()) == R"(
first
second
)" + 1);
}

TEST_CASE("shadow_and_local")
{
    vessel_str_buf_clear();

    vessel_interpret("test", R"(
{
  var a = "outer"
  {
    print a // expect: outer
    var a = "inner"
    print a // expect: inner
  }
}
)");
    REQUIRE(std::string(vessel_get_str_buf()) == R"(
outer
inner
)" + 1);
}

TEST_CASE("shadow_global")
{
    vessel_str_buf_clear();

    vessel_interpret("test", R"(
var a = "global"
{
  var a = "shadow"
  print a // expect: shadow
}
print a // expect: global
)");
    REQUIRE(std::string(vessel_get_str_buf()) == R"(
shadow
global
)" + 1);
}

TEST_CASE("shadow_local")
{
    vessel_str_buf_clear();

    vessel_interpret("test", R"(
{
  var a = "local"
  {
    var a = "shadow"
    print a // expect: shadow
  }
  print a // expect: local
}
)");
    REQUIRE(std::string(vessel_get_str_buf()) == R"(
shadow
local
)" + 1);
}

TEST_CASE("uninitialized")
{
    vessel_str_buf_clear();

    vessel_interpret("test", R"(
var a
print a // expect: nil
)");
    REQUIRE(std::string(vessel_get_str_buf()) == R"(
nil
)" + 1);
}

TEST_CASE("unreached_undefined")
{
    vessel_str_buf_clear();

    vessel_interpret("test", R"(
if (false) {
  print notDefined
}

print "ok" // expect: ok
)");
    REQUIRE(std::string(vessel_get_str_buf()) == R"(
ok
)" + 1);
}

TEST_CASE("use_global_in_initializer")
{
    vessel_str_buf_clear();

    vessel_interpret("test", R"(
var a = "value"
var a = a
print a // expect: value
)");
    REQUIRE(std::string(vessel_get_str_buf()) == R"(
value
)" + 1);
}
