#include "utility.h"

#include <catch/catch.hpp>

#include <vessel.h>

TEST_CASE("arguments")
{
    init_output_buf();

    ves_interpret("test", R"(
class Foo {
  init(a, b) {
    System.print("init") // expect: init
    this.a = a
    this.b = b
  }
}

var foo = Foo(1, 2)
System.print(foo.a) // expect: 1
System.print(foo.b) // expect: 2
)");
    REQUIRE(std::string(get_output_buf()) == R"(
init
1
2
)" + 1);
}

TEST_CASE("call_init_early_return")
{
    init_output_buf();

    ves_interpret("test", R"(
class Foo {
  init() {
    System.print("init")
    return
    System.print("nope")
  }
}

var foo = Foo() // expect: init
System.print(foo.init()) // expect: init
// expect: Foo instance
)");
    REQUIRE(std::string(get_output_buf()) == R"(
init
init
Foo instance
)" + 1);
}

TEST_CASE("call_init_explicitly")
{
    init_output_buf();

    ves_interpret("test", R"foo(
class Foo {
  init(arg) {
    System.print("Foo.init(" + arg + ")")
    this.field = "init"
}
}

var foo = Foo("one") // expect: Foo.init(one)
foo.field = "field"

var foo2 = foo.init("two") // expect: Foo.init(two)
System.print(foo2) // expect: Foo instance

// Make sure init() doesn't create a fresh instance.
System.print(foo.field) // expect: init
)foo");
    REQUIRE(std::string(get_output_buf()) == R"(
Foo.init(one)
Foo.init(two)
Foo instance
init
)" + 1);
}

TEST_CASE("default")
{
    init_output_buf();

    ves_interpret("test", R"(
class Foo {}

var foo = Foo()
System.print(foo) // expect: Foo instance
)");
    REQUIRE(std::string(get_output_buf()) == R"(
Foo instance
)" + 1);
}

TEST_CASE("early_return")
{
    init_output_buf();

    ves_interpret("test", R"(
class Foo {
  init() {
    System.print("init")
    return
    System.print("nope")
  }
}

var foo = Foo() // expect: init
System.print(foo) // expect: Foo instance
)");
    REQUIRE(std::string(get_output_buf()) == R"(
init
Foo instance
)" + 1);
}

TEST_CASE("init_not_method")
{
    init_output_buf();

    ves_interpret("test", R"foo(
class Foo {
  init(arg) {
    System.print("Foo.init(" + arg + ")")
    this.field = "init"
}
}

fun init() {
    System.print("not initializer")
}

init() // expect: not initializer
)foo");
    REQUIRE(std::string(get_output_buf()) == R"(
not initializer
)" + 1);
}

TEST_CASE("return_in_nested_function")
{
    init_output_buf();

    ves_interpret("test", R"(
class Foo {
  init() {
    fun init() {
      return "bar"
    }
    System.print(init()) // expect: bar
  }
}

System.print(Foo()) // expect: Foo instance
)");
    REQUIRE(std::string(get_output_buf()) == R"(
bar
Foo instance
)" + 1);
}