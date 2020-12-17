#include <catch/catch.hpp>

#include <vm.h>
#include <debug.h>

TEST_CASE("equality")
{
    ves_str_buf_clear();

    interpret(R"(
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
    REQUIRE(std::string(ves_get_str_buf()) == R"(
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
    ves_str_buf_clear();

    interpret(R"(
print !true    // expect: false
print !false   // expect: true
print !!true   // expect: true
)");
    REQUIRE(std::string(ves_get_str_buf()) == R"(
false
true
true
)" + 1);
}
