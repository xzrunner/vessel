#include "utility.h"

#include <catch/catch.hpp>

#include <vessel.h>

TEST_CASE("literals")
{
    init_output_buf();

    ves_interpret("test", R"(
System.print(123)     // expect: 123
System.print(987654)  // expect: 987654
System.print(0)       // expect: 0
System.print(-0)      // expect: -0

System.print(123.456) // expect: 123.456
System.print(-0.001)  // expect: -0.001
)");
    REQUIRE(std::string(get_output_buf()) == R"(
123
987654
0
-0
123.456
-0.001
)" + 1);
}

TEST_CASE("nan_equality")
{
    init_output_buf();

    ves_interpret("test", R"(
var nan = 0/0

System.print(nan == 0) // expect: false
System.print(nan != 1) // expect: true

// NaN is not equal to self.
System.print(nan == nan) // expect: false
System.print(nan != nan) // expect: true
)");
    REQUIRE(std::string(get_output_buf()) == R"(
false
true
false
true
)" + 1);
}