#include "utility.h"

#include <catch/catch.hpp>

#include <vessel.h>

TEST_CASE("abs")
{
    init_output_buf();

    ves_interpret("test", R"(
import "math" for Math

System.print(Math.abs(123))
System.print(Math.abs(-123))
System.print(Math.abs(0))
System.print(Math.abs(-0))
System.print(Math.abs(-0.12))
System.print(Math.abs(12.34))
)");
    REQUIRE(std::string(get_output_buf()) == R"(
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
    init_output_buf();

    ves_interpret("test", R"(
import "math" for Math

System.print(Math.acos(0))
System.print(Math.acos(1))
System.print(Math.acos(-1))
)");
    REQUIRE(std::string(get_output_buf()) == R"(
1.5707963267949
0
3.1415926535898
)" + 1);
}

TEST_CASE("asin")
{
    init_output_buf();

    ves_interpret("test", R"(
import "math" for Math

System.print(Math.asin(0))
System.print(Math.asin(1))
System.print(Math.asin(-1))
)");
    REQUIRE(std::string(get_output_buf()) == R"(
0
1.5707963267949
-1.5707963267949
)" + 1);
}

TEST_CASE("atan")
{
    init_output_buf();

    ves_interpret("test", R"(
import "math" for Math

System.print(Math.atan(0))
System.print(Math.atan(1))
)");
    REQUIRE(std::string(get_output_buf()) == R"(
0
0.78539816339745
)" + 1);
}

TEST_CASE("ceil")
{
    init_output_buf();

    ves_interpret("test", R"(
import "math" for Math

System.print(Math.ceil(123))
System.print(Math.ceil(-123))
System.print(Math.ceil(0))
System.print(Math.ceil(-0))
System.print(Math.ceil(0.123))
System.print(Math.ceil(12.3))
System.print(Math.ceil(-0.123))
System.print(Math.ceil(-12.3))
)");
    REQUIRE(std::string(get_output_buf()) == R"(
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
    init_output_buf();

    ves_interpret("test", R"(
import "math" for Math

System.print(Math.cos(0))                                  // expect: 1
System.print(Math.cos(Math.pi()))                          // expect: -1
System.print(Math.cos(2 * Math.pi()))                      // expect: 1
System.print(Math.abs(Math.cos(Math.pi() / 2)) < 0.000001) // expect: true
)");
    REQUIRE(std::string(get_output_buf()) == R"(
1
-1
1
true
)" + 1);
}

TEST_CASE("floor")
{
    init_output_buf();

    ves_interpret("test", R"(
import "math" for Math

System.print(Math.floor(123))
System.print(Math.floor(-123))
System.print(Math.floor(0))
System.print(Math.floor(-0))
System.print(Math.floor(0.123))
System.print(Math.floor(12.3))
System.print(Math.floor(-0.123))
System.print(Math.floor(-12.3))
)");
    REQUIRE(std::string(get_output_buf()) == R"(
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
    init_output_buf();

    ves_interpret("test", R"(
import "math" for Math

System.print(Math.round(123))
System.print(Math.round(-123))
System.print(Math.round(0))
System.print(Math.round(-0))
System.print(Math.round(0.123))
System.print(Math.round(12.3))
System.print(Math.round(-0.123))
System.print(Math.round(-12.3))
)");
    REQUIRE(std::string(get_output_buf()) == R"(
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
    init_output_buf();

    ves_interpret("test", R"(
import "math" for Math

System.print(Math.sin(0))              // expect: 0
System.print(Math.sin(Math.pi() / 2))  // expect: 1

// these should of course be 0, but it's not that precise
System.print(Math.abs(Math.sin(Math.pi())) < 0.0000000001)        // expect: true
System.print(Math.abs(Math.sin(Math.pi() * 2)) < 0.0000000001)    // expect: true
)");
    REQUIRE(std::string(get_output_buf()) == R"(
0
1
true
true
)" + 1);
}

TEST_CASE("sqrt")
{
    init_output_buf();

    ves_interpret("test", R"(
import "math" for Math

System.print(Math.sqrt(4))
System.print(Math.sqrt(1000000))
System.print(Math.sqrt(1))
System.print(Math.sqrt(-0))
System.print(Math.sqrt(0))
System.print(Math.sqrt(2))
)");
    REQUIRE(std::string(get_output_buf()) == R"(
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
    init_output_buf();

    ves_interpret("test", R"(
import "math" for Math

System.print(Math.tan(0))               // expect: 0
System.print(Math.tan(Math.pi() / 4))   // expect: 1
System.print(Math.tan(- Math.pi() / 4)) // expect: -1

)");
    REQUIRE(std::string(get_output_buf()) == R"(
0
1
-1
)" + 1);
}

TEST_CASE("log")
{
    init_output_buf();

    ves_interpret("test", R"(
import "math" for Math

System.print(Math.log(3))
System.print(Math.log(100))
System.print(Math.log(-1))
)");
    REQUIRE(std::string(get_output_buf()) == R"(
1.0986122886681
4.6051701859881
-nan(ind)
)" + 1);
}

TEST_CASE("log2")
{
    init_output_buf();

    ves_interpret("test", R"(
import "math" for Math

System.print(Math.log2(1024))
System.print(Math.log2(2048))
System.print(Math.log2(100))
System.print(Math.log2(-1))
)");
    REQUIRE(std::string(get_output_buf()) == R"(
10
11
6.6438561897747
nan
)" + 1);
}

TEST_CASE("exp")
{
    init_output_buf();

    ves_interpret("test", R"(
import "math" for Math

System.print(Math.exp(5))
System.print(Math.exp(10))
System.print(Math.exp(-1))
)");
    REQUIRE(std::string(get_output_buf()) == R"(
148.41315910258
22026.465794807
0.36787944117144
)" + 1);
}