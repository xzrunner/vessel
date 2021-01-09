#include <catch/catch.hpp>

#include <vessel.h>

TEST_CASE("equality")
{
    vessel_str_buf_clear();

    vessel_interpret("test", R"(
print true == true    // expect: true
print true == false   // expect: false
print false == true   // expect: false
print false == false  // expect: true

// Not equal to other types.
print true == 1        // expect: false
print false == 0       // expect: false
print true == "true"   // expect: false
print false == "false" // expect: false
print false == ""      // expect: false

print true != true    // expect: false
print true != false   // expect: true
print false != true   // expect: true
print false != false  // expect: false

// Not equal to other types.
print true != 1        // expect: true
print false != 0       // expect: true
print true != "true"   // expect: true
print false != "false" // expect: true
print false != ""      // expect: true
)");
    REQUIRE(std::string(vessel_get_str_buf()) == R"(
true
false
false
true
false
false
false
false
false
false
true
true
false
true
true
true
true
true
)" + 1);
}

TEST_CASE("bool_equality")
{
    vessel_str_buf_clear();

    vessel_interpret("test", R"(
print !true    // expect: false
print !false   // expect: true
print !!true   // expect: true
)");
    REQUIRE(std::string(vessel_get_str_buf()) == R"(
false
true
true
)" + 1);
}

TEST_CASE("bool_type")
{
    vessel_str_buf_clear();

    vessel_interpret("test", R"(
print(true is Bool)      // expect: true
print(true is Object)    // expect: true
print(true is Num)       // expect: false
print(true.type == Bool) // expect: true
)");
    REQUIRE(std::string(vessel_get_str_buf()) == R"(
true
true
false
true
)" + 1);
}
