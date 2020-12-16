#include <catch/catch.hpp>

#include <vm.h>
#include <debug.h>

TEST_CASE("call_function_field")
{
    ves_str_buf_clear();

    interpret(R"(
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

    interpret(R"(
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

