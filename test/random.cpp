#include <catch/catch.hpp>

#include <vessel.h>

TEST_CASE("float")
{
    vessel_str_buf_clear();

    vessel_interpret("test", R"(
import "random" for Random

var random = Random.init(12345)

var below = 0
for (var i = 1; i < 1000; i = i + 1) {
  var n = random.float()
  if (n < 0.5) below = below + 1
}

// Should be roughly evenly distributed.
print(below > 450) // expect: true
print(below < 550) // expect: true
)");
    REQUIRE(std::string(vessel_get_str_buf()) == R"(
true
true
)" + 1);
}

TEST_CASE("float_max")
{
    vessel_str_buf_clear();

    vessel_interpret("test", R"(
import "random" for Random

var random = Random.init(12345)

var below = 0
for (var i = 1; i < 100; i = i + 1) {
  var n = random.float(5)
  if (n < 0) print("too low")
  if (n >= 5) print("too high")
}
)");
    REQUIRE(std::string(vessel_get_str_buf()) == R"(
)" + 1);
}

TEST_CASE("float_min_max")
{
    vessel_str_buf_clear();

    vessel_interpret("test", R"(
import "random" for Random

var random = Random.init(12345)

var below = 0
for (var i = 1; i < 100; i = i + 1) {
  var n = random.float(2, 5)
  if (n < 2) print("too low")
  if (n >= 5) print("too high")
}
)");
    REQUIRE(std::string(vessel_get_str_buf()) == R"(
)" + 1);
}

TEST_CASE("int")
{
    vessel_str_buf_clear();

    vessel_interpret("test", R"(
import "random" for Random

var random = Random.init(12345)

var below = 0
for (var i = 1; i < 1000; i = i + 1) {
  var n = random.int()
  if (n < 2147483648) below = below + 1
}

// Should be roughly evenly distributed.
print(below > 450) // expect: true
print(below < 550) // expect: true
)");
    REQUIRE(std::string(vessel_get_str_buf()) == R"(
true
true
)" + 1);
}

//TEST_CASE("int_max")
//{
//    vessel_str_buf_clear();
//
//    vessel_interpret("test", R"(
//import "random" for Random
//
//var random = Random.init(12345)
//
//var counts = [0, 0, 0, 0, 0]
//for (var i = 1; i < 10000; i = i + 1) {
//  var n = random.int(5)
//  counts[n] = counts[n] + 1
//}
//
//for (var i = 1; i < counts.count; i = i + 1) {
//  if (counts[i] < 1900) print("too few")
//  if (counts[i] > 2100) print("too many")
//}
//)");
//    REQUIRE(std::string(vessel_get_str_buf()) == R"(
//)" + 1);
//}