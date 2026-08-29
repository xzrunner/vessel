#include "utility.h"

#include <catch2/catch_test_macros.hpp>

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

TEST_CASE("scientific_notation")
{
    init_output_buf();

    REQUIRE(ves_interpret("scientific_notation", R"(
System.print(1e-3 == 0.001)
System.print(2E+3 == 2000)
System.print(-4.1799442055467e-17 == -0.000000000000000041799442055467)
)") == VES_INTERPRET_OK);
    REQUIRE(std::string(get_output_buf()) == R"(
true
true
true
)" + 1);
}

TEST_CASE("invalid_scientific_notation_keeps_token_boundaries")
{
    REQUIRE(ves_interpret("number_then_identifier", "System.print(12edges)") ==
            VES_INTERPRET_COMPILE_ERROR);
    REQUIRE(ves_interpret("missing_exponent_digits", "System.print(1e+)") ==
            VES_INTERPRET_COMPILE_ERROR);
    REQUIRE(ves_interpret("identifier_after_e", "System.print(1efoo)") ==
            VES_INTERPRET_COMPILE_ERROR);
}
