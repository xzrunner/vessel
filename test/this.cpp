#include "utility.h"

#include <catch/catch.hpp>

#include <vessel.h>

TEST_CASE("this-closure")
{
    init_output_buf();

    ves_interpret("test", R"(
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
System.print(closure()) // expect: Foo
)");
    REQUIRE(std::string(get_output_buf()) == R"(
Foo
)" + 1);
}

TEST_CASE("nested_class")
{
    init_output_buf();

    ves_interpret("test", R"(
class Outer {
  method() {
    System.print(this) // expect: Outer instance

    fun f() {
      System.print(this) // expect: Outer instance

      class Inner {
        method() {
          System.print(this) // expect: Inner instance
        }
      }

      Inner().method()
    }
    f()
  }
}

Outer().method()
)");
    REQUIRE(std::string(get_output_buf()) == R"(
Outer instance
Outer instance
Inner instance
)" + 1);
}

TEST_CASE("this_nested_closure")
{
    init_output_buf();

    ves_interpret("test", R"(
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
System.print(closure()()()) // expect: Foo
)");
    REQUIRE(std::string(get_output_buf()) == R"(
Foo
)" + 1);
}

TEST_CASE("this_in_method")
{
    init_output_buf();

    ves_interpret("test", R"(
class Foo {
  bar() { return this }
  baz() { return "baz" }
}

System.print(Foo().bar().baz()) // expect: baz
)");
    REQUIRE(std::string(get_output_buf()) == R"(
baz
)" + 1);
}