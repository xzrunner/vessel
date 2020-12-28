#include <catch/catch.hpp>

#include <vessel.h>

TEST_CASE("call_function_field")
{
    ves_str_buf_clear();

    interpret("test", R"(
class Foo {}

fun bar(a, b) {
  print "bar"
  print a
  print b
}

var foo = Foo()
foo.bar = bar

foo.bar(1, 2)
// expect: bar
// expect: 1
// expect: 2
)");
    REQUIRE(std::string(ves_get_str_buf()) == R"(
bar
1
2
)" + 1);
}

TEST_CASE("get_and_set_method")
{
    ves_str_buf_clear();

    interpret("test", R"(
// Bound methods have identity equality.
class Foo {
  method(a) {
    print "method"
    print a
  }
  other(a) {
    print "other"
    print a
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
    REQUIRE(std::string(ves_get_str_buf()) == R"(
other
1
method
2
)" + 1);
}

TEST_CASE("method")
{
    ves_str_buf_clear();

    interpret("test", R"(
class Foo {
  bar(arg) {
    print arg
  }
}

var bar = Foo().bar
print "got method" // expect: got method
bar("arg")          // expect: arg
)");
    REQUIRE(std::string(ves_get_str_buf()) == R"(
got method
arg
)" + 1);
}

TEST_CASE("method_binds_this")
{
    ves_str_buf_clear();

    interpret("test", R"(
class Foo {
  sayName(a) {
    print this.name
    print a
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
    REQUIRE(std::string(ves_get_str_buf()) == R"(
foo1
1
)" + 1);
}

TEST_CASE("on_instance")
{
    ves_str_buf_clear();

    interpret("test", R"(
class Foo {}

var foo = Foo()

print foo.bar = "bar value" // expect: bar value
print foo.baz = "baz value" // expect: baz value

print foo.bar // expect: bar value
print foo.baz // expect: baz value
)");
    REQUIRE(std::string(ves_get_str_buf()) == R"(
bar value
baz value
bar value
baz value
)" + 1);
}