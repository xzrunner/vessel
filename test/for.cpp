#include <catch/catch.hpp>

#include <vm.h>
#include <debug.h>

TEST_CASE("closure_in_body")
{
    ves_str_buf_clear();

    interpret(R"(
var f1
var f2
var f3

for (var i = 1; i < 4; i = i + 1) {
  var j = i
  fun f() {
    print i
    print j
  }

  if (j == 1) f1 = f
  else if (j == 2) f2 = f
  else f3 = f
}

f1() // expect: 4
     // expect: 1
f2() // expect: 4
     // expect: 2
f3() // expect: 4
     // expect: 3
)");
    REQUIRE(std::string(ves_get_str_buf()) == R"(
4
1
4
2
4
3
)" + 1);
}

