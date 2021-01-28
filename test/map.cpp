#include "utility.h"

#include <catch/catch.hpp>

#include <vessel.h>

//TEST_CASE("map_reuse")
//{
//    init_output_buf();
//
//    ves_interpret("test", R"(
//var map = {}
//map[2] = "two"
//map[0] = "zero"
//map.remove(2)
//map[0] = "zero again"
//map.remove(0)
//
//System.print(map.containsKey(0)) // expect: false
//)");
//    REQUIRE(std::string(get_output_buf()) == R"(
//false
//)" + 1);
//}

//TEST_CASE("map_remove")
//{
//    init_output_buf();
//
//    ves_interpret("test", R"(
//var map = {
//  "one": 1,
//  "two": 2,
//  "three": 3
//}
//
//System.print(map.count) // expect: 3
//System.print(map.remove("two")) // expect: 2
//System.print(map.count) // expect: 2
//System.print(map.remove("three")) // expect: 3
//System.print(map.count) // expect: 1
//
//// Remove an already removed entry.
//System.print(map.remove("two")) // expect: null
//System.print(map.count) // expect: 1
//
//System.print(map.remove("one")) // expect: 1
//System.print(map.count) // expect: 0
//)");
//    REQUIRE(std::string(get_output_buf()) == R"(
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
    init_output_buf();

    ves_interpret("test", R"(
var map = {
  "one": 1,
  "two": 2,
  "three": 3
}

System.print(map.containsKey("one")) // expect: true
System.print(map.containsKey("two")) // expect: true
System.print(map.containsKey("three")) // expect: true
System.print(map.containsKey("four")) // expect: false
System.print(map.containsKey("five")) // expect: false
)");
    REQUIRE(std::string(get_output_buf()) == R"(
true
true
true
false
false
)" + 1);
}