#include <catch/catch.hpp>

#include <vm.h>
#include <debug.h>

TEST_CASE("multiline")
{
    ves_str_buf_clear();

    interpret(NULL, R"(
var a = "1
2
3"
print a
// expect: 1
// expect: 2
// expect: 3
)");
    REQUIRE(std::string(ves_get_str_buf()) == R"(
1
2
3
)" + 1);
}

