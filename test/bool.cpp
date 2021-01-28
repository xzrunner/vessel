#include "utility.h"

#include <catch/catch.hpp>

#include <vessel.h>

TEST_CASE("equality")
{
    init_output_buf();

    ves_interpret("test", R"(
System.print(true == true)    // expect: true
System.print(true == false)   // expect: false
System.print(false == true)   // expect: false
System.print(false == false)  // expect: true

// Not equal to other types.
System.print(true == 1)        // expect: false
System.print(false == 0)       // expect: false
System.print(true == "true")   // expect: false
System.print(false == "false") // expect: false
System.print(false == "")      // expect: false

System.print(true != true)    // expect: false
System.print(true != false)   // expect: true
System.print(false != true)   // expect: true
System.print(false != false)  // expect: false

// Not equal to other types.
System.print(true != 1)        // expect: true
System.print(false != 0)       // expect: true
System.print(true != "true")   // expect: true
System.print(false != "false") // expect: true
System.print(false != "")      // expect: true
)");
    REQUIRE(std::string(get_output_buf()) == R"(
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
    init_output_buf();

    ves_interpret("test", R"(
System.print(!true)    // expect: false
System.print(!false)   // expect: true
System.print(!!true)   // expect: true
)");
    REQUIRE(std::string(get_output_buf()) == R"(
false
true
true
)" + 1);
}

TEST_CASE("bool_type")
{
    init_output_buf();

    ves_interpret("test", R"(
System.print(true is Bool)      // expect: true
System.print(true is Object)    // expect: true
System.print(true is Num)       // expect: false
System.print(true.type == Bool) // expect: true
)");
    REQUIRE(std::string(get_output_buf()) == R"(
true
true
false
true
)" + 1);
}