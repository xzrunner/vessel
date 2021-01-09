#include <catch/catch.hpp>

#include <vessel.h>

TEST_CASE("line_at_eof")
{
    vessel_str_buf_clear();

    vessel_interpret("test", R"(
print "ok" // expect: ok
// comment
)");
    REQUIRE(std::string(vessel_get_str_buf()) == R"(
ok
)" + 1);
}

TEST_CASE("only_line_comment")
{
    vessel_str_buf_clear();

    vessel_interpret("test", R"(
// comment
)");
}

TEST_CASE("only_line_comment_and_line")
{
    vessel_str_buf_clear();

    vessel_interpret("test", R"(
// comment

)");
}
