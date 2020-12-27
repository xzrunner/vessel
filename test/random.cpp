#include <catch/catch.hpp>

#include <vessel.h>

TEST_CASE("float")
{
    printf("++++++++++++++++++++++++++ random\n");

    ves_str_buf_clear();

    interpret(NULL, R"(
import "random" for Random

var random = Random.new(12345)

var below = 0
for (var i = 1; i < 1000; i = i + 1) {
  var n = random.float()
  if (n < 0.5) below = below + 1
}

// Should be roughly evenly distributed.
print(below > 450) // expect: true
print(below < 550) // expect: true
)");
    REQUIRE(std::string(ves_get_str_buf()) == R"(
true
true
)" + 1);
}
