#include <catch/catch.hpp>

#include <vm.h>
#include <debug.h>

TEST_CASE("assign_to_closure")
{
    ves_str_buf_clear();

    interpret(R"(
var f
var g

{
  var local = "local"
  fun f_() {
    print local
    local = "after f"
    print local
  }
  f = f_

  fun g_() {
    print local
    local = "after g"
    print local
  }
  g = g_
}

f()
// expect: local
// expect: after f

g()
// expect: after f
// expect: after g
)");
    REQUIRE(std::string(ves_get_str_buf()) == R"(
local
after f
after f
after g
)" + 1);
}

TEST_CASE("nested_closure")
{
    ves_str_buf_clear();

    interpret(R"(
var f

fun f1() {
  var a = "a"
  fun f2() {
    var b = "b"
    fun f3() {
      var c = "c"
      fun f4() {
        print a
        print b
        print c
      }
      f = f4
    }
    f3()
  }
  f2()
}
f1()

f()
// expect: a
// expect: b
// expect: c
)");
    REQUIRE(std::string(ves_get_str_buf()) == R"(
a
b
c
)" + 1);
}

