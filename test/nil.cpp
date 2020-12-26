#include <catch/catch.hpp>

#include <vessel.h>

TEST_CASE("literal")
{
    ves_str_buf_clear();

    interpret(NULL, R"(
print nil // expect: nil
)");
    REQUIRE(std::string(ves_get_str_buf()) == R"(
nil
)" + 1);
}

