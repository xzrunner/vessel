#include <catch/catch.hpp>

#include <vessel.h>

TEST_CASE("class_empty")
{
    vessel_str_buf_clear();

    vessel_interpret("test", R"(
class Foo {}

print Foo // expect: Foo
)");
    REQUIRE(std::string(vessel_get_str_buf()) == R"(
Foo
)" + 1);
}

TEST_CASE("inherited_method")
{
    vessel_str_buf_clear();

    vessel_interpret("test", R"(
class Foo {
  inFoo() {
    print "in foo"
  }
}

class Bar < Foo {
  inBar() {
    print "in bar"
  }
}

class Baz < Bar {
  inBaz() {
    print "in baz"
  }
}

var baz = Baz()
baz.inFoo() // expect: in foo
baz.inBar() // expect: in bar
baz.inBaz() // expect: in baz
)");
    REQUIRE(std::string(vessel_get_str_buf()) == R"(
in foo
in bar
in baz
)" + 1);
}

TEST_CASE("local_inherit_other")
{
    vessel_str_buf_clear();

    vessel_interpret("test", R"(
class A {}

fun f() {
  class B < A {}
  return B
}

print f() // expect: B
)");
    REQUIRE(std::string(vessel_get_str_buf()) == R"(
B
)" + 1);
}

TEST_CASE("local_reference_self")
{
    vessel_str_buf_clear();

    vessel_interpret("test", R"(
{
  class Foo {
    returnSelf() {
      return Foo
    }
  }

  print Foo().returnSelf() // expect: Foo
}
)");
    REQUIRE(std::string(vessel_get_str_buf()) == R"(
Foo
)" + 1);
}

TEST_CASE("reference_self")
{
    vessel_str_buf_clear();

    vessel_interpret("test", R"(
class Foo {
  returnSelf() {
    return Foo
  }
}

print Foo().returnSelf() // expect: Foo
)");
    REQUIRE(std::string(vessel_get_str_buf()) == R"(
Foo
)" + 1);
}