#include <catch/catch.hpp>

#include <vm.h>
#include <debug.h>

TEST_CASE("inherited_method")
{
    ves_str_buf_clear();

    interpret(R"(
class Foo {
  inFoo() {
    print "in foo"
  }
}

class Bar < Foo {
  inBar() {
    print "in bar"
  }
}

class Baz < Bar {
  inBaz() {
    print "in baz"
  }
}

var baz = Baz()
baz.inFoo() // expect: in foo
baz.inBar() // expect: in bar
baz.inBaz() // expect: in baz
)");
    REQUIRE(std::string(ves_get_str_buf()) == R"(
in foo
in bar
in baz
)" + 1);
}

