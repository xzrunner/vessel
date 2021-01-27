#include <catch/catch.hpp>

#include <vessel.h>

TEST_CASE("line_at_eof")
{
    ves_str_buf_clear();

    ves_interpret("test", R"(
print "ok" // expect: ok
// comment
)");
    REQUIRE(std::string(ves_get_str_buf()) == R"(
ok
)" + 1);
}

TEST_CASE("only_line_comment")
{
    ves_str_buf_clear();

    ves_interpret("test", R"(
// comment
)");
}

TEST_CASE("only_line_comment_and_line")
{
    ves_str_buf_clear();

    ves_interpret("test", R"(
// comment

)");
}
