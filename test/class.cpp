#include "utility.h"

#include <catch/catch.hpp>

#include <vessel.h>

TEST_CASE("class_empty")
{
    init_output_buf();

    ves_interpret("test", R"(
class Foo {}

System.print(Foo) // expect: Foo
)");
    REQUIRE(std::string(get_output_buf()) == R"(
Foo
)" + 1);
}

TEST_CASE("inherited_method")
{
    init_output_buf();

    ves_interpret("test", R"(
class Foo {
  inFoo() {
    System.print("in foo")
  }
}

class Bar is Foo {
  inBar() {
    System.print("in bar")
  }
}

class Baz is Bar {
  inBaz() {
    System.print("in baz")
  }
}

var baz = Baz()
baz.inFoo() // expect: in foo
baz.inBar() // expect: in bar
baz.inBaz() // expect: in baz
)");
    REQUIRE(std::string(get_output_buf()) == R"(
in foo
in bar
in baz
)" + 1);
}

TEST_CASE("local_inherit_other")
{
    init_output_buf();

    ves_interpret("test", R"(
class A {}

fun f() {
  class B is A {}
  return B
}

System.print(f()) // expect: B
)");
    REQUIRE(std::string(get_output_buf()) == R"(
B
)" + 1);
}

TEST_CASE("local_reference_self")
{
    init_output_buf();

    ves_interpret("test", R"(
{
  class Foo {
    returnSelf() {
      return Foo
    }
  }

  System.print(Foo().returnSelf()) // expect: Foo
}
)");
    REQUIRE(std::string(get_output_buf()) == R"(
Foo
)" + 1);
}

TEST_CASE("reference_self")
{
    init_output_buf();

    ves_interpret("test", R"(
class Foo {
  returnSelf() {
    return Foo
  }
}

System.print(Foo().returnSelf()) // expect: Foo
)");
    REQUIRE(std::string(get_output_buf()) == R"(
Foo
)" + 1);
}