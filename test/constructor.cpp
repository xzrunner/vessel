#include <catch/catch.hpp>

#include <vm.h>
#include <debug.h>

TEST_CASE("arguments")
{
    ves_str_buf_clear();

    interpret(R"(
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

