#include "utility.h"

#include <catch/catch.hpp>

#include <vessel.h>

TEST_CASE("early_bound")
{
    init_output_buf();

    ves_interpret("test", R"(
var a = "outer"
{
  fun foo() {
    System.print(a)
  }

  foo() // expect: outer
  var a = "inner"
  foo() // expect: outer
}
)");
    REQUIRE(std::string(get_output_buf()) == R"(
outer
outer
)" + 1);
}

TEST_CASE("in_middle_of_block")
{
    init_output_buf();

    ves_interpret("test", R"(
{
  var a = "a"
  System.print(a) // expect: a
  var b = a + " b"
  System.print(b) // expect: a b
  var c = a + " c"
  System.print(c) // expect: a c
  var d = b + " d"
  System.print(d) // expect: a b d
}
)");
    REQUIRE(std::string(get_output_buf()) == R"(
a
a b
a c
a b d
)" + 1);
}

TEST_CASE("in_nested_block")
{
    init_output_buf();

    ves_interpret("test", R"(
{
  var a = "outer"
  {
    System.print(a) // expect: outer
  }
}
)");
    REQUIRE(std::string(get_output_buf()) == R"(
outer
)" + 1);
}

TEST_CASE("local_from_method")
{
    init_output_buf();

    ves_interpret("test", R"(
var foo = "variable"

class Foo {
  method() {
    System.print(foo)
  }
}

Foo().method() // expect: variable
)");
    REQUIRE(std::string(get_output_buf()) == R"(
variable
)" + 1);
}

TEST_CASE("redeclare_global")
{
    init_output_buf();

    ves_interpret("test", R"(
var a = "1"
var a
System.print(a) // expect: nil
)");
    REQUIRE(std::string(get_output_buf()) == R"(
nil
)" + 1);
}

TEST_CASE("redefine_global")
{
    init_output_buf();

    ves_interpret("test", R"(
var a = "1"
var a = "2"
System.print(a) // expect: 2
)");
    REQUIRE(std::string(get_output_buf()) == R"(
2
)" + 1);
}

TEST_CASE("scope_reuse_in_different_blocks")
{
    init_output_buf();

    ves_interpret("test", R"(
{
  var a = "first"
  System.print(a) // expect: first
}

{
  var a = "second"
  System.print(a) // expect: second
}
)");
    REQUIRE(std::string(get_output_buf()) == R"(
first
second
)" + 1);
}

TEST_CASE("shadow_and_local")
{
    init_output_buf();

    ves_interpret("test", R"(
{
  var a = "outer"
  {
    System.print(a) // expect: outer
    var a = "inner"
    System.print(a) // expect: inner
  }
}
)");
    REQUIRE(std::string(get_output_buf()) == R"(
outer
inner
)" + 1);
}

TEST_CASE("shadow_global")
{
    init_output_buf();

    ves_interpret("test", R"(
var a = "global"
{
  var a = "shadow"
  System.print(a) // expect: shadow
}
System.print(a) // expect: global
)");
    REQUIRE(std::string(get_output_buf()) == R"(
shadow
global
)" + 1);
}

TEST_CASE("shadow_local")
{
    init_output_buf();

    ves_interpret("test", R"(
{
  var a = "local"
  {
    var a = "shadow"
    System.print(a) // expect: shadow
  }
  System.print(a) // expect: local
}
)");
    REQUIRE(std::string(get_output_buf()) == R"(
shadow
local
)" + 1);
}

TEST_CASE("uninitialized")
{
    init_output_buf();

    ves_interpret("test", R"(
var a
System.print(a) // expect: nil
)");
    REQUIRE(std::string(get_output_buf()) == R"(
nil
)" + 1);
}

TEST_CASE("unreached_undefined")
{
    init_output_buf();

    ves_interpret("test", R"(
if (false) {
  System.print(notDefined)
}

System.print("ok") // expect: ok
)");
    REQUIRE(std::string(get_output_buf()) == R"(
ok
)" + 1);
}

TEST_CASE("use_global_in_initializer")
{
    init_output_buf();

    ves_interpret("test", R"(
var a = "value"
var a = a
System.print(a) // expect: value
)");
    REQUIRE(std::string(get_output_buf()) == R"(
value
)" + 1);
}