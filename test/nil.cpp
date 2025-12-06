#include "utility.h"

#include <catch2/catch_test_macros.hpp>

#include <vessel.h>

TEST_CASE("literal")
{
    init_output_buf();

    ves_interpret("test", R"(
System.print(nil) // expect: nil
)");
    REQUIRE(std::string(get_output_buf()) == R"(
nil
)" + 1);
}