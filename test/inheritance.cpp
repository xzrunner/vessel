#include "utility.h"

#include <catch/catch.hpp>

#include <vessel.h>

TEST_CASE("inheritance_constructor")
{
    init_output_buf();

    ves_interpret("test", R"(
class A {
  init(param) {
    this.field = param
  }

  test() {
    System.print(this.field)
  }
}

class B is A {}

var b = B("value")
b.test() // expect: value
)");
    REQUIRE(std::string(get_output_buf()) == R"(
value
)" + 1);
}

TEST_CASE("inherit_methods")
{
    init_output_buf();

    ves_interpret("test", R"(
class Foo {
  methodOnFoo() { System.print("foo") }
  override() { System.print("foo") }
}

class Bar is Foo {
  methodOnBar() { System.print("bar") }
  override() { System.print("bar") }
}

var bar = Bar()
bar.methodOnFoo() // expect: foo
bar.methodOnBar() // expect: bar
bar.override() // expect: bar
)");
    REQUIRE(std::string(get_output_buf()) == R"(
foo
bar
bar
)" + 1);
}

TEST_CASE("set_fields_from_base_class")
{
    init_output_buf();

    ves_interpret("test", R"(
class Foo {
  foo(a, b) {
    this.field1 = a
    this.field2 = b
  }

  fooPrint() {
    System.print(this.field1)
    System.print(this.field2)
  }
}

class Bar is Foo {
  bar(a, b) {
    this.field1 = a
    this.field2 = b
  }

  barPrint() {
    System.print(this.field1)
    System.print(this.field2)
  }
}

var bar = Bar()
bar.foo("foo 1", "foo 2")
bar.fooPrint()
// expect: foo 1
// expect: foo 2

bar.bar("bar 1", "bar 2")
bar.barPrint()
// expect: bar 1
// expect: bar 2

bar.fooPrint()
// expect: bar 1
// expect: bar 2
)");
    REQUIRE(std::string(get_output_buf()) == R"(
foo 1
foo 2
bar 1
bar 2
bar 1
bar 2
)" + 1);
}