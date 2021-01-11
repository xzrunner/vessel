#include <catch/catch.hpp>

#include <vessel.h>

TEST_CASE("abs")
{
    vessel_str_buf_clear();

    vessel_interpret("test", R"(
import "math" for Math

print(Math.abs(123))
print(Math.abs(-123))
print(Math.abs(0))
print(Math.abs(-0))
print(Math.abs(-0.12))
print(Math.abs(12.34))
)");
    REQUIRE(std::string(vessel_get_str_buf()) == R"(
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
    vessel_str_buf_clear();

    vessel_interpret("test", R"(
import "math" for Math

print(Math.acos(0))
print(Math.acos(1))
print(Math.acos(-1))
)");
    REQUIRE(std::string(vessel_get_str_buf()) == R"(
1.5707963267949
0
3.1415926535898
)" + 1);
}

TEST_CASE("asin")
{
    vessel_str_buf_clear();

    vessel_interpret("test", R"(
import "math" for Math

print(Math.asin(0))
print(Math.asin(1))
print(Math.asin(-1))
)");
    REQUIRE(std::string(vessel_get_str_buf()) == R"(
0
1.5707963267949
-1.5707963267949
)" + 1);
}

TEST_CASE("atan")
{
    vessel_str_buf_clear();

    vessel_interpret("test", R"(
import "math" for Math

print(Math.atan(0))
print(Math.atan(1))
)");
    REQUIRE(std::string(vessel_get_str_buf()) == R"(
0
0.78539816339745
)" + 1);
}

TEST_CASE("ceil")
{
    vessel_str_buf_clear();

    vessel_interpret("test", R"(
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
    REQUIRE(std::string(vessel_get_str_buf()) == R"(
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

//// todo Math's const
//TEST_CASE("cos")
//{
//    vessel_str_buf_clear();
//
//    vessel_interpret("test", R"(
//import "math" for Math
//
//System.print(0.cos)             // expect: 1
//System.print(Num.pi.cos)        // expect: -1
//System.print((2 * Num.pi).cos)  // expect: 1
//System.print((Num.pi / 2).cos.abs < 1.0e-16) // expect: true
//)");
//    REQUIRE(std::string(vessel_get_str_buf()) == R"(
//)" + 1);
//}

TEST_CASE("floor")
{
    vessel_str_buf_clear();

    vessel_interpret("test", R"(
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
    REQUIRE(std::string(vessel_get_str_buf()) == R"(
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
    vessel_str_buf_clear();

    vessel_interpret("test", R"(
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
    REQUIRE(std::string(vessel_get_str_buf()) == R"(
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

//// todo Math's const
//TEST_CASE("sin")
//{
//    vessel_str_buf_clear();
//
//    vessel_interpret("test", R"(
//import "math" for Math
//
//System.print(0.sin)             // expect: 0
//System.print((Num.pi / 2).sin)  // expect: 1
//
//// these should of course be 0, but it's not that precise
//System.print(Num.pi.sin.abs < 1.0e-15)        // expect: true
//System.print((2 * Num.pi).sin.abs < 1.0e-15)  // expect: true
//)");
//    REQUIRE(std::string(vessel_get_str_buf()) == R"(
//0
//0.78539816339745
//)" + 1);
//}

TEST_CASE("sqrt")
{
    vessel_str_buf_clear();

    vessel_interpret("test", R"(
import "math" for Math

print(Math.sqrt(4))
print(Math.sqrt(1000000))
print(Math.sqrt(1))
print(Math.sqrt(-0))
print(Math.sqrt(0))
print(Math.sqrt(2))
)");
    REQUIRE(std::string(vessel_get_str_buf()) == R"(
2
1000
1
-0
0
1.4142135623731
)" + 1);
}

//TEST_CASE("tan")
//{
//    vessel_str_buf_clear();
//
//    vessel_interpret("test", R"(
//import "math" for Math
//
//System.print(0.tan)             // expect: 0
//System.print((Num.pi / 4).tan)  // expect: 1
//System.print((-Num.pi / 4).tan) // expect: -1
//
//)");
//    REQUIRE(std::string(vessel_get_str_buf()) == R"(
//
//)" + 1);
//}

TEST_CASE("log")
{
    vessel_str_buf_clear();

    vessel_interpret("test", R"(
import "math" for Math

print(Math.log(3))
print(Math.log(100))
print(Math.log(-1))
)");
    REQUIRE(std::string(vessel_get_str_buf()) == R"(
1.0986122886681
4.6051701859881
-nan(ind)
)" + 1);
}

TEST_CASE("log2")
{
    vessel_str_buf_clear();

    vessel_interpret("test", R"(
import "math" for Math

print(Math.log2(1024))
print(Math.log2(2048))
print(Math.log2(100))
print(Math.log2(-1))
)");
    REQUIRE(std::string(vessel_get_str_buf()) == R"(
10
11
6.6438561897747
nan
)" + 1);
}

TEST_CASE("exp")
{
    vessel_str_buf_clear();

    vessel_interpret("test", R"(
import "math" for Math

print(Math.exp(5))
print(Math.exp(10))
print(Math.exp(-1))
)");
    REQUIRE(std::string(vessel_get_str_buf()) == R"(
148.41315910258
22026.465794807
0.36787944117144
)" + 1);
}