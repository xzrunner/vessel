#include <catch/catch.hpp>

#include <vm.h>
#include <debug.h>

TEST_CASE("list_add")
{
    ves_str_buf_clear();

    interpret(NULL, R"(
{
var a = [1]
a.add(2)
print(a) // expect: [1, 2]
a.add(3)
print(a) // expect: [1, 2, 3]

// Returns added element.
print(a.add(4)) // expect: 4
}
)");
    REQUIRE(std::string(ves_get_str_buf()) == R"(
[1, 2]
[1, 2, 3]
4
)" + 1);
}
