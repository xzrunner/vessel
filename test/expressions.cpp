#include <catch/catch.hpp>

#include <vm.h>
#include <debug.h>

TEST_CASE("evaluate")
{
    ves_str_buf_clear();

    interpret(R"(
// Note: This is just for the expression evaluating chapter which evaluates an
// expression directly.
(5 - (3 - 1)) + -1
// expect: 2
)");
//    REQUIRE(std::string(ves_get_str_buf()) == R"(
//2
//)" + 1);
}

TEST_CASE("parse")
{
    ves_str_buf_clear();

    interpret(R"(
// Note: This is just for the expression parsing chapter which prints the AST.
(5 - (3 - 1)) + -1
// expect: (+ (group (- 5.0 (group (- 3.0 1.0)))) (- 1.0))
)");
}