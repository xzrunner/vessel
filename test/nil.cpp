#include <catch/catch.hpp>

#include <vessel.h>

TEST_CASE("literal")
{
    vessel_str_buf_clear();

    vessel_interpret("test", R"(
print nil // expect: nil
)");
    REQUIRE(std::string(vessel_get_str_buf()) == R"(
nil
)" + 1);
}

