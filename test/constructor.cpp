#include <catch/catch.hpp>

#include <vessel.h>

TEST_CASE("arguments")
{
    ves_str_buf_clear();

    ves_interpret("test", R"(
class Foo {
  init(a, b) {
    print "init" // expect: init
    this.a = a
    this.b = b
  }
}

var foo = Foo(1, 2)
print foo.a // expect: 1
print foo.b // expect: 2
)");
    REQUIRE(std::string(ves_get_str_buf()) == R"(
init
1
2
)" + 1);
}

TEST_CASE("call_init_early_return")
{
    ves_str_buf_clear();

    ves_interpret("test", R"(
class Foo {
  init() {
    print "init"
    return
    print "nope"
  }
}

var foo = Foo() // expect: init
print foo.init() // expect: init
// expect: Foo instance
)");
    REQUIRE(std::string(ves_get_str_buf()) == R"(
init
init
Foo instance
)" + 1);
}

TEST_CASE("call_init_explicitly")
{
    ves_str_buf_clear();

    ves_interpret("test", R"foo(
class Foo {
  init(arg) {
    print "Foo.init(" + arg + ")"
    this.field = "init"
}
}

var foo = Foo("one") // expect: Foo.init(one)
foo.field = "field"

var foo2 = foo.init("two") // expect: Foo.init(two)
print foo2 // expect: Foo instance

// Make sure init() doesn't create a fresh instance.
print foo.field // expect: init
)foo");
    REQUIRE(std::string(ves_get_str_buf()) == R"(
Foo.init(one)
Foo.init(two)
Foo instance
init
)" + 1);
}

TEST_CASE("default")
{
    ves_str_buf_clear();

    ves_interpret("test", R"(
class Foo {}

var foo = Foo()
print foo // expect: Foo instance
)");
    REQUIRE(std::string(ves_get_str_buf()) == R"(
Foo instance
)" + 1);
}

TEST_CASE("early_return")
{
    ves_str_buf_clear();

    ves_interpret("test", R"(
class Foo {
  init() {
    print "init"
    return
    print "nope"
  }
}

var foo = Foo() // expect: init
print foo // expect: Foo instance
)");
    REQUIRE(std::string(ves_get_str_buf()) == R"(
init
Foo instance
)" + 1);
}

TEST_CASE("init_not_method")
{
    ves_str_buf_clear();

    ves_interpret("test", R"foo(
class Foo {
  init(arg) {
    print "Foo.init(" + arg + ")"
    this.field = "init"
}
}

fun init() {
    print "not initializer"
}

init() // expect: not initializer
)foo");
    REQUIRE(std::string(ves_get_str_buf()) == R"(
not initializer
)" + 1);
}

TEST_CASE("return_in_nested_function")
{
    ves_str_buf_clear();

    ves_interpret("test", R"(
class Foo {
  init() {
    fun init() {
      return "bar"
    }
    print init() // expect: bar
  }
}

print Foo() // expect: Foo instance
)");
    REQUIRE(std::string(ves_get_str_buf()) == R"(
bar
Foo instance
)" + 1);
}