#include <catch/catch.hpp>

#include <vm.h>
#include <debug.h>

TEST_CASE("while_closure_in_body")
{
    ves_str_buf_clear();

    interpret(R"(
var f1
var f2
var f3

var i = 1
while (i < 4) {
  var j = i
  fun f() { print j }

  if (j == 1) f1 = f
  else if (j == 2) f2 = f
  else f3 = f

  i = i + 1
}

f1() // expect: 1
f2() // expect: 2
f3() // expect: 3
)");
    REQUIRE(std::string(ves_get_str_buf()) == R"(
1
2
3
)" + 1);
}

TEST_CASE("while_return_closure")
{
    ves_str_buf_clear();

    interpret(R"(
fun f() {
  while (true) {
    var i = "i"
    fun g() { print i }
    return g
  }
}

var h = f()
h() // expect: i
)");
    REQUIRE(std::string(ves_get_str_buf()) == R"(
i
)" + 1);
}

TEST_CASE("while_return_inside")
{
    ves_str_buf_clear();

    interpret(R"(
fun f() {
  while (true) {
    var i = "i"
    return i
  }
}

print f()
// expect: i
)");
    REQUIRE(std::string(ves_get_str_buf()) == R"(
i
)" + 1);
}

TEST_CASE("while_syntax")
{
    ves_str_buf_clear();

    interpret(R"(
// Single-expression body.
var c = 0
while (c < 3) print c = c + 1
// expect: 1
// expect: 2
// expect: 3

// Block body.
var a = 0
while (a < 3) {
  print a
  a = a + 1
}
// expect: 0
// expect: 1
// expect: 2

// Statement bodies.
while (false) if (true) 1 else 2
while (false) while (true) 1
while (false) for (;;) 1
)");
    REQUIRE(std::string(ves_get_str_buf()) == R"(
1
2
3
0
1
2
)" + 1);
}
