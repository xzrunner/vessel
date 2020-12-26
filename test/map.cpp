#include <catch/catch.hpp>

#include <vm.h>
#include <debug.h>

//TEST_CASE("map_reuse")
//{
//    ves_str_buf_clear();
//
//    interpret(NULL, R"(
//var map = {}
//map[2] = "two"
//map[0] = "zero"
//map.remove(2)
//map[0] = "zero again"
//map.remove(0)
//
//print(map.containsKey(0)) // expect: false
//)");
//    REQUIRE(std::string(ves_get_str_buf()) == R"(
//false
//)" + 1);
//}

//TEST_CASE("map_remove")
//{
//    ves_str_buf_clear();
//
//    interpret(NULL, R"(
//var map = {
//  "one": 1,
//  "two": 2,
//  "three": 3
//}
//
//print(map.count) // expect: 3
//print(map.remove("two")) // expect: 2
//print(map.count) // expect: 2
//print(map.remove("three")) // expect: 3
//print(map.count) // expect: 1
//
//// Remove an already removed entry.
//print(map.remove("two")) // expect: null
//print(map.count) // expect: 1
//
//print(map.remove("one")) // expect: 1
//print(map.count) // expect: 0
//)");
//    REQUIRE(std::string(ves_get_str_buf()) == R"(
//3
//2
//2
//3
//1
//null
//1
//1
//0
//)" + 1);
//}

TEST_CASE("contains_key")
{
    ves_str_buf_clear();

    interpret(NULL, R"(
var map = {
  "one": 1,
  "two": 2,
  "three": 3
}

print(map.containsKey("one")) // expect: true
print(map.containsKey("two")) // expect: true
print(map.containsKey("three")) // expect: true
print(map.containsKey("four")) // expect: false
print(map.containsKey("five")) // expect: false
)");
    REQUIRE(std::string(ves_get_str_buf()) == R"(
true
true
true
false
false
)" + 1);
}
