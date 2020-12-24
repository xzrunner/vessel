#include <catch/catch.hpp>

#include <vm.h>
#include <debug.h>

TEST_CASE("line_at_eof")
{
    ves_str_buf_clear();

    interpret(NULL, R"(
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

    interpret(NULL, R"(
// comment
)");
}

TEST_CASE("only_line_comment_and_line")
{
    ves_str_buf_clear();

    interpret(NULL, R"(
// comment

)");
}
