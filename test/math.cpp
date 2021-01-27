#include <catch/catch.hpp>

#include <vessel.h>

TEST_CASE("abs")
{
    ves_str_buf_clear();

    ves_interpret("test", R"(
import "math" for Math

print(Math.abs(123))
print(Math.abs(-123))
print(Math.abs(0))
print(Math.abs(-0))
print(Math.abs(-0.12))
print(Math.abs(12.34))
)");
    REQUIRE(std::string(ves_get_str_buf()) == R"(
123
123
0
0
0.12
12.34
)" + 1);
}

TEST_CASE("acos")
{
    ves_str_buf_clear();

    ves_interpret("test", R"(
import "math" for Math

print(Math.acos(0))
print(Math.acos(1))
print(Math.acos(-1))
)");
    REQUIRE(std::string(ves_get_str_buf()) == R"(
1.5707963267949
0
3.1415926535898
)" + 1);
}

TEST_CASE("asin")
{
    ves_str_buf_clear();

    ves_interpret("test", R"(
import "math" for Math

print(Math.asin(0))
print(Math.asin(1))
print(Math.asin(-1))
)");
    REQUIRE(std::string(ves_get_str_buf()) == R"(
0
1.5707963267949
-1.5707963267949
)" + 1);
}

TEST_CASE("atan")
{
    ves_str_buf_clear();

    ves_interpret("test", R"(
import "math" for Math

print(Math.atan(0))
print(Math.atan(1))
)");
    REQUIRE(std::string(ves_get_str_buf()) == R"(
0
0.78539816339745
)" + 1);
}

TEST_CASE("ceil")
{
    ves_str_buf_clear();

    ves_interpret("test", R"(
import "math" for Math

print(Math.ceil(123))
print(Math.ceil(-123))
print(Math.ceil(0))
print(Math.ceil(-0))
print(Math.ceil(0.123))
print(Math.ceil(12.3))
print(Math.ceil(-0.123))
print(Math.ceil(-12.3))
)");
    REQUIRE(std::string(ves_get_str_buf()) == R"(
123
-123
0
-0
1
13
-0
-12
)" + 1);
}

TEST_CASE("cos")
{
    ves_str_buf_clear();

    ves_interpret("test", R"(
import "math" for Math

print(Math.cos(0))                                  // expect: 1
print(Math.cos(Math.pi()))                          // expect: -1
print(Math.cos(2 * Math.pi()))                      // expect: 1
print(Math.abs(Math.cos(Math.pi() / 2)) < 0.000001) // expect: true
)");
    REQUIRE(std::string(ves_get_str_buf()) == R"(
1
-1
1
true
)" + 1);
}

TEST_CASE("floor")
{
    ves_str_buf_clear();

    ves_interpret("test", R"(
import "math" for Math

print(Math.floor(123))
print(Math.floor(-123))
print(Math.floor(0))
print(Math.floor(-0))
print(Math.floor(0.123))
print(Math.floor(12.3))
print(Math.floor(-0.123))
print(Math.floor(-12.3))
)");
    REQUIRE(std::string(ves_get_str_buf()) == R"(
123
-123
0
-0
0
12
-1
-13
)" + 1);
}

TEST_CASE("round")
{
    ves_str_buf_clear();

    ves_interpret("test", R"(
import "math" for Math

print(Math.round(123))
print(Math.round(-123))
print(Math.round(0))
print(Math.round(-0))
print(Math.round(0.123))
print(Math.round(12.3))
print(Math.round(-0.123))
print(Math.round(-12.3))
)");
    REQUIRE(std::string(ves_get_str_buf()) == R"(
123
-123
0
-0
0
12
-0
-12
)" + 1);
}

TEST_CASE("sin")
{
    ves_str_buf_clear();

    ves_interpret("test", R"(
import "math" for Math

print(Math.sin(0))              // expect: 0
print(Math.sin(Math.pi() / 2))  // expect: 1

// these should of course be 0, but it's not that precise
print(Math.abs(Math.sin(Math.pi())) < 0.0000000001)        // expect: true
print(Math.abs(Math.sin(Math.pi() * 2)) < 0.0000000001)    // expect: true
)");
    REQUIRE(std::string(ves_get_str_buf()) == R"(
0
1
true
true
)" + 1);
}

TEST_CASE("sqrt")
{
    ves_str_buf_clear();

    ves_interpret("test", R"(
import "math" for Math

print(Math.sqrt(4))
print(Math.sqrt(1000000))
print(Math.sqrt(1))
print(Math.sqrt(-0))
print(Math.sqrt(0))
print(Math.sqrt(2))
)");
    REQUIRE(std::string(ves_get_str_buf()) == R"(
2
1000
1
-0
0
1.4142135623731
)" + 1);
}

TEST_CASE("tan")
{
    ves_str_buf_clear();

    ves_interpret("test", R"(
import "math" for Math

print(Math.tan(0))               // expect: 0
print(Math.tan(Math.pi() / 4))   // expect: 1
print(Math.tan(- Math.pi() / 4)) // expect: -1

)");
    REQUIRE(std::string(ves_get_str_buf()) == R"(
0
1
-1
)" + 1);
}

TEST_CASE("log")
{
    ves_str_buf_clear();

    ves_interpret("test", R"(
import "math" for Math

print(Math.log(3))
print(Math.log(100))
print(Math.log(-1))
)");
    REQUIRE(std::string(ves_get_str_buf()) == R"(
1.0986122886681
4.6051701859881
-nan(ind)
)" + 1);
}

TEST_CASE("log2")
{
    ves_str_buf_clear();

    ves_interpret("test", R"(
import "math" for Math

print(Math.log2(1024))
print(Math.log2(2048))
print(Math.log2(100))
print(Math.log2(-1))
)");
    REQUIRE(std::string(ves_get_str_buf()) == R"(
10
11
6.6438561897747
nan
)" + 1);
}

TEST_CASE("exp")
{
    ves_str_buf_clear();

    ves_interpret("test", R"(
import "math" for Math

print(Math.exp(5))
print(Math.exp(10))
print(Math.exp(-1))
)");
    REQUIRE(std::string(ves_get_str_buf()) == R"(
148.41315910258
22026.465794807
0.36787944117144
)" + 1);
}