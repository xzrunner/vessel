#include <catch/catch.hpp>

#include <vm.h>
#include <debug.h>

TEST_CASE("this-closure")
{
    ves_str_buf_clear();

    interpret(R"(
class Foo {
  getClosure() {
    fun closure() {
      return this.toString()
    }
    return closure
  }

  toString() { return "Foo" }
}

var closure = Foo().getClosure()
print closure() // expect: Foo
)");
    REQUIRE(std::string(ves_get_str_buf()) == R"(
Foo
)" + 1);
}

TEST_CASE("nested_class")
{
    ves_str_buf_clear();

    interpret(R"(
class Outer {
  method() {
    print this // expect: Outer instance

    fun f() {
      print this // expect: Outer instance

      class Inner {
        method() {
          print this // expect: Inner instance
        }
      }

      Inner().method()
    }
    f()
  }
}

Outer().method()
)");
    REQUIRE(std::string(ves_get_str_buf()) == R"(
Outer instance
Outer instance
Inner instance
)" + 1);
}

TEST_CASE("this_nested_closure")
{
    ves_str_buf_clear();

    interpret(R"(
class Foo {
  getClosure() {
    fun f() {
      fun g() {
        fun h() {
          return this.toString()
        }
        return h
      }
      return g
    }
    return f
  }

  toString() { return "Foo" }
}

var closure = Foo().getClosure()
print closure()()() // expect: Foo
)");
    REQUIRE(std::string(ves_get_str_buf()) == R"(
Foo
)" + 1);
}

TEST_CASE("this_in_method")
{
    ves_str_buf_clear();

    interpret(R"(
class Foo {
  bar() { return this }
  baz() { return "baz" }
}

print Foo().bar().baz() // expect: baz
)");
    REQUIRE(std::string(ves_get_str_buf()) == R"(
baz
)" + 1);
}
