#include <catch/catch.hpp>

#include <vessel.h>

TEST_CASE("multiline")
{
    vessel_str_buf_clear();

    vessel_interpret("test", R"(
var a = "1
2
3"
print a
// expect: 1
// expect: 2
// expect: 3
)");
    REQUIRE(std::string(vessel_get_str_buf()) == R"(
1
2
3
)" + 1);
}

