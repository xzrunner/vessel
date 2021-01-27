#include <catch/catch.hpp>

#include <vessel.h>

TEST_CASE("float")
{
    ves_str_buf_clear();

    ves_interpret("test", R"(
import "random" for Random

var random = Random.init(12345)

var below = 0
for (var i in 1..=1000) {
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

TEST_CASE("float_max")
{
    ves_str_buf_clear();

    ves_interpret("test", R"(
import "random" for Random

var random = Random.init(12345)

var below = 0
for (var i in 1..=100) {
  var n = random.float(5)
  if (n < 0) print("too low")
  if (n >= 5) print("too high")
}
)");
    REQUIRE(std::string(ves_get_str_buf()) == R"(
)" + 1);
}

TEST_CASE("float_min_max")
{
    ves_str_buf_clear();

    ves_interpret("test", R"(
import "random" for Random

var random = Random.init(12345)

var below = 0
for (var i in 1..=100) {
  var n = random.float(2, 5)
  if (n < 2) print("too low")
  if (n >= 5) print("too high")
}
)");
    REQUIRE(std::string(ves_get_str_buf()) == R"(
)" + 1);
}

TEST_CASE("int")
{
    ves_str_buf_clear();

    ves_interpret("test", R"(
import "random" for Random

var random = Random.init(12345)

var below = 0
for (var i in 1..=1000) {
  var n = random.int()
  if (n < 2147483648) below = below + 1
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

TEST_CASE("int_max")
{
    ves_str_buf_clear();

    ves_interpret("test", R"(
import "random" for Random

var random = Random.init(12345)

var counts = [0, 0, 0, 0, 0]
for (var i in 1..=10000) {
  var n = random.int(5)
  counts[n] = counts[n] + 1
}

for (var count in counts) {
  if (count < 1900) print("too few")
  if (count > 2100) print("too many")
}
)");
    REQUIRE(std::string(ves_get_str_buf()) == R"(
)" + 1);
}

TEST_CASE("int_min_max")
{
    ves_str_buf_clear();

    ves_interpret("test", R"(
import "random" for Random

var random = Random.init(12345)

var counts = [0, 0, 0, 0, 0, 0, 0, 0]
for (var i in 1..=10000) {
  var n = random.int(3, 8)
  counts[n] = counts[n] + 1
}

for (var i in 0..=2) {
  if (counts[i] != 0) print("too low value")
}

for (var i in 3..=7) {
  if (counts[i] < 1900) print("too few")
  if (counts[i] > 2100) print("too many")
}
)");
    REQUIRE(std::string(ves_get_str_buf()) == R"(
)" + 1);
}

TEST_CASE("random_new")
{
    ves_str_buf_clear();

    ves_interpret("test", R"(
import "random" for Random

var random = Random.init()

var correct = 0
for (var i in 1..=100) {
  var n = random.float()
  if (n >= 0) correct = correct + 1
  if (n < 1) correct = correct + 1
}

print(correct) // expect: 200
)");
    REQUIRE(std::string(ves_get_str_buf()) == R"(
200
)" + 1);
}

TEST_CASE("new_number")
{
    ves_str_buf_clear();

    ves_interpret("test", R"(
import "random" for Random

var random1 = Random.init(123)
var random2 = Random.init(123)

var correct = 0
for (var i in 1..=100) {
  // Should produce the same values.
  if (random1.float() == random2.float()) correct = correct + 1
}

print(correct) // expect: 100
)");
    REQUIRE(std::string(ves_get_str_buf()) == R"(
100
)" + 1);
}

TEST_CASE("new_sequence")
{
    ves_str_buf_clear();

    ves_interpret("test", R"(
import "random" for Random

var random1 = Random.init([1, 2, 3])
var random2 = Random.init([1, 2, 3])

var correct = 0
for (var i in 1..=100) {
  // Should produce the same values.
  if (random1.float() == random2.float()) correct = correct + 1
}

print(correct) // expect: 100
)");
    REQUIRE(std::string(ves_get_str_buf()) == R"(
100
)" + 1);
}

//TEST_CASE("sample_count_multiple")
//{
//    ves_str_buf_clear();
//
//    ves_interpret("test", R"foo(
//import "random" for Random
//
//var random = Random.new(12345)
//
//// Should choose all elements with roughly equal probability.
//var list = (0...10).toList
//var binom = [1, 10, 45, 120, 210, 252, 210, 120, 45, 10, 1]
//
//for (var k in 0..=10) {
//  var count = binom[k]
//
//  var histogram = {}
//  for (var i in 1..=count * 100) {
//    var sample = random.sample(list, k)
//    // Create a bitmask to represent the unordered set.
//    var bitmask = 0
//    sample.each {|s| bitmask = bitmask | (1 << s) }
//    if (!histogram.containsKey(bitmask)) histogram[bitmask] = 0
//    histogram[bitmask] = histogram[bitmask] + 1
//  }
//
//  if (histogram.count != count) print("!!! %(count) %(histogram.count)")
//  for (key in histogram.keys) {
//    var error = (histogram[key] - 100).abs
//    if (error > 50) print("!!! %(error)")
//  }
//}
//)foo");
//    REQUIRE(std::string(ves_get_str_buf()) == R"(
//100
//)" + 1);
//}

//TEST_CASE("sample_count_one")
//{
//    ves_str_buf_clear();
//
//    ves_interpret("test", R"foo(
//import "random" for Random
//
//var random = Random.init(12345)
//
//// Single element list.
//print(random.sample(["single"], 1)) // expect: [single]
//
//// Should choose all elements with roughly equal probability.
//var list = ["a", "b", "c", "d", "e"]
//var histogram = {}
//for (var i in 1..=5000) {
//  var sample = random.sample(list, 1)
//
//  var string = sample.toString
//  if (!histogram.containsKey(string)) histogram[string] = 0
//  histogram[string] = histogram[string] + 1
//}
//
//print(histogram.count) // expect: 5
//for (var key in histogram) {
//  import "math" for Math
//  var error = Math.abs((histogram[key.toString] / (5000 / list.count) - 1))
//  if (error > 0.1) print("!!! %(error)")
//}
//)foo");
//    REQUIRE(std::string(ves_get_str_buf()) == R"(
//[single]
//5
//)" + 1);
//}

TEST_CASE("sample_count_zero")
{
    ves_str_buf_clear();

    ves_interpret("test", R"(
import "random" for Random

var random = Random.init(12345)

print(random.sample([], 0)) // expect: []
print(random.sample([1, 2, 3], 0)) // expect: []
)");
    REQUIRE(std::string(ves_get_str_buf()) == R"(
[]
[]
)" + 1);
}

TEST_CASE("sample_one")
{
    ves_str_buf_clear();

    ves_interpret("test", R"foo(
import "random" for Random

var random = Random.init(12345)

// Single element list.
print(random.sample(["single"])) // expect: single

// Should choose all elements with roughly equal probability.
var list = ["a", "b", "c", "d", "e"]
var histogram = {"a": 0, "b": 0, "c": 0, "d": 0, "e": 0}
for (var i in 1..1000) {
  var sample = random.sample(list)
  histogram[sample] = histogram[sample] + 1
}

print(histogram.count) // expect: 5
for (var key in histogram) {
  import "math" for Math
  var error = Math.abs(histogram[key] / (1000 / list.count) - 1)
  if (error > 0.2) print("!!! %(error)")
}
)foo");
    REQUIRE(std::string(ves_get_str_buf()) == R"(
single
5
)" + 1);
}

//TEST_CASE("shuffle")
//{
//    ves_str_buf_clear();
//
//    ves_interpret("test", R"foo(
//import "random" for Random
//
//var random = Random.init(12345)
//
//// Empty list.
//var list = []
//random.shuffle(list)
//print(list) // expect: []
//
//// One element.
//list = [1]
//random.shuffle(list)
//print(list) // expect: [1]
//
//// Given enough tries, should generate all permutations with roughly equal
//// probability.
//var histogram = {}
//for (var i in 1..5000) {
//  var list = [1, 2, 3, 4]
//  random.shuffle(list)
//
//  var string = list.toString
//  if (!histogram.containsKey(string)) histogram[string] = 0
//  histogram[string] = histogram[string] + 1
//}
//
//print(histogram.count) // expect: 24
//for (var key in histogram.keys) {
//  import "math" for Math
//  var error = Math.abs(histogram[key] / (5000 / 24) - 1)
//  if (error > 0.21) print("!!! %(error)")
//}
//)foo");
//    REQUIRE(std::string(ves_get_str_buf()) == R"(
//[]
//[1]
//24
//)" + 1);
//}