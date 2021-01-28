#include "utility.h"

#include <catch/catch.hpp>

#include <vessel.h>

TEST_CASE("call_function_field")
{
    init_output_buf();

    ves_interpret("test", R"(
class Foo {}

fun bar(a, b) {
  System.print("bar")
  System.print(a)
  System.print(b)
}

var foo = Foo()
foo.bar = bar

foo.bar(1, 2)
// expect: bar
// expect: 1
// expect: 2
)");
    REQUIRE(std::string(get_output_buf()) == R"(
bar
1
2
)" + 1);
}

TEST_CASE("get_and_set_method")
{
    init_output_buf();

    ves_interpret("test", R"(
// Bound methods have identity equality.
class Foo {
  method(a) {
    System.print("method")
    System.print(a)
  }
  other(a) {
    System.print("other")
    System.print(a)
  }
}

var foo = Foo()
var method = foo.method

// Setting a property shadows the instance method.
foo.method = foo.other
foo.method(1)
// expect: other
// expect: 1

// The old method handle still points to the original method.
method(2)
// expect: method
// expect: 2
)");
    REQUIRE(std::string(get_output_buf()) == R"(
other
1
method
2
)" + 1);
}

TEST_CASE("method")
{
    init_output_buf();

    ves_interpret("test", R"(
class Foo {
  bar(arg) {
    System.print(arg)
  }
}

var bar = Foo().bar
System.print("got method") // expect: got method
bar("arg")          // expect: arg
)");
    REQUIRE(std::string(get_output_buf()) == R"(
got method
arg
)" + 1);
}

TEST_CASE("method_binds_this")
{
    init_output_buf();

    ves_interpret("test", R"(
class Foo {
  sayName(a) {
    System.print(this.name)
    System.print(a)
  }
}

var foo1 = Foo()
foo1.name = "foo1"

var foo2 = Foo()
foo2.name = "foo2"

// Store the method reference on another object.
foo2.fn = foo1.sayName
// Still retains original receiver.
foo2.fn(1)
// expect: foo1
// expect: 1
)");
    REQUIRE(std::string(get_output_buf()) == R"(
foo1
1
)" + 1);
}

TEST_CASE("on_instance")
{
    init_output_buf();

    ves_interpret("test", R"(
class Foo {}

var foo = Foo()

System.print(foo.bar = "bar value") // expect: bar value
System.print(foo.baz = "baz value") // expect: baz value

System.print(foo.bar) // expect: bar value
System.print(foo.baz) // expect: baz value
)");
    REQUIRE(std::string(get_output_buf()) == R"(
bar value
baz value
bar value
baz value
)" + 1);
}