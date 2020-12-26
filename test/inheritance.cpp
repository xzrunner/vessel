#include <catch/catch.hpp>

#include <vessel.h>

TEST_CASE("inheritance_constructor")
{
    ves_str_buf_clear();

    interpret(NULL, R"(
class A {
  init(param) {
    this.field = param
  }

  test() {
    print this.field
  }
}

class B < A {}

var b = B("value")
b.test() // expect: value
)");
    REQUIRE(std::string(ves_get_str_buf()) == R"(
value
)" + 1);
}

TEST_CASE("inherit_methods")
{
    ves_str_buf_clear();

    interpret(NULL, R"(
class Foo {
  methodOnFoo() { print "foo" }
  override() { print "foo" }
}

class Bar < Foo {
  methodOnBar() { print "bar" }
  override() { print "bar" }
}

var bar = Bar()
bar.methodOnFoo() // expect: foo
bar.methodOnBar() // expect: bar
bar.override() // expect: bar
)");
    REQUIRE(std::string(ves_get_str_buf()) == R"(
foo
bar
bar
)" + 1);
}

TEST_CASE("set_fields_from_base_class")
{
    ves_str_buf_clear();

    interpret(NULL, R"(
class Foo {
  foo(a, b) {
    this.field1 = a
    this.field2 = b
  }

  fooPrint() {
    print this.field1
    print this.field2
  }
}

class Bar < Foo {
  bar(a, b) {
    this.field1 = a
    this.field2 = b
  }

  barPrint() {
    print this.field1
    print this.field2
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
    REQUIRE(std::string(ves_get_str_buf()) == R"(
foo 1
foo 2
bar 1
bar 2
bar 1
bar 2
)" + 1);
}
