#include <catch/catch.hpp>

#include <vessel.h>

TEST_CASE("literals")
{
    ves_str_buf_clear();

    interpret("test", R"(
print 123     // expect: 123
print 987654  // expect: 987654
print 0       // expect: 0
print -0      // expect: -0

print 123.456 // expect: 123.456
print -0.001  // expect: -0.001
)");
    REQUIRE(std::string(ves_get_str_buf()) == R"(
123
987654
0
-0
123.456
-0.001
)" + 1);
}

TEST_CASE("nan_equality")
{
    ves_str_buf_clear();

    interpret("test", R"(
var nan = 0/0

print nan == 0 // expect: false
print nan != 1 // expect: true

// NaN is not equal to self.
print nan == nan // expect: false
print nan != nan // expect: true
)");
    REQUIRE(std::string(ves_get_str_buf()) == R"(
false
true
false
true
)" + 1);
}

