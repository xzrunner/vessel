#include "utility.h"

#include <catch/catch.hpp>

#include <vessel.h>

TEST_CASE("multiline")
{
    init_output_buf();

    ves_interpret("test", R"(
var a = "1
2
3"
System.print(a)
// expect: 1
// expect: 2
// expect: 3
)");
    REQUIRE(std::string(get_output_buf()) == R"(
1
2
3
)" + 1);
}