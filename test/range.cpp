#include <catch/catch.hpp>

#include <vessel.h>

TEST_CASE("range")
{
    vessel_str_buf_clear();

    vessel_interpret("test", R"(
print(11..22)
)");
    REQUIRE(std::string(vessel_get_str_buf()) == R"(
range(11, 22)
)" + 1);
}