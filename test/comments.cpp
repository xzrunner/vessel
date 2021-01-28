#include "utility.h"

#include <catch/catch.hpp>

#include <vessel.h>

TEST_CASE("line_at_eof")
{
    init_output_buf();

    ves_interpret("test", R"(
System.print("ok") // expect: ok
// comment
)");
    REQUIRE(std::string(get_output_buf()) == R"(
ok
)" + 1);
}

TEST_CASE("only_line_comment")
{
    init_output_buf();

    ves_interpret("test", R"(
// comment
)");
}

TEST_CASE("only_line_comment_and_line")
{
    init_output_buf();

    ves_interpret("test", R"(
// comment

)");
}