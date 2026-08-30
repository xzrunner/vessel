#include "utility.h"

#include <catch2/catch_test_macros.hpp>

#include <vessel.h>

#include <cstdint>
#include <string>

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
nan
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

TEST_CASE("numeric_specials")
{
    init_output_buf();

    REQUIRE(ves_interpret("test", R"(
System.print(0/0)
System.print(1/0)
System.print(-1/0)
System.print(-(0/0))
System.print(123.456)
System.print((0/0).toString())
System.print((1/0).toString())
System.print((-1/0).toString())
)") == VES_INTERPRET_OK);
    REQUIRE(std::string(get_output_buf()) == R"(
nan
infinity
-infinity
nan
123.456
nan
infinity
-infinity
)" + 1);
}

TEST_CASE("to_pointer_full_width_identity")
{
    init_output_buf();

    REQUIRE(ves_interpret("test", R"(
class Node {}
var a = Node()
var b = Node()
System.print(a.to_pointer())
System.print(a.to_pointer())
System.print(b.to_pointer())
)") == VES_INTERPRET_OK);

    const std::string out(get_output_buf());
    const auto nl1 = out.find('\n');
    REQUIRE(nl1 != std::string::npos);
    const auto nl2 = out.find('\n', nl1 + 1);
    REQUIRE(nl2 != std::string::npos);
    const auto nl3 = out.find('\n', nl2 + 1);
    REQUIRE(nl3 != std::string::npos);
    REQUIRE(nl3 + 1 == out.size());

    const std::string a1 = out.substr(0, nl1);
    const std::string a2 = out.substr(nl1 + 1, nl2 - nl1 - 1);
    const std::string b = out.substr(nl2 + 1, nl3 - nl2 - 1);

    REQUIRE(a1 == a2);
    REQUIRE(a1 != b);
    REQUIRE(!a1.empty());
    REQUIRE(!b.empty());
    REQUIRE(a1.find_first_not_of("0123456789") == std::string::npos);
    REQUIRE(b.find_first_not_of("0123456789") == std::string::npos);

    const unsigned long long ua = std::stoull(a1);
    const unsigned long long ub = std::stoull(b);
    REQUIRE(ua != ub);
#if UINTPTR_MAX > 0xFFFFFFFFu
    const auto legacy_low32_decimal = [](unsigned long long value) {
        const uint32_t low = static_cast<uint32_t>(value);
        if (low <= 0x7FFFFFFFu) {
            return std::to_string(static_cast<unsigned long long>(low));
        }
        return std::to_string(static_cast<long long>(low) - 0x100000000ll);
    };
    // The old %d implementation exposed only the signed low 32 bits. A
    // low-address allocator can make that text legitimately equal to the
    // full pointer, so apply the truncation comparison only when high bits
    // are actually present instead of requiring ASLR to place objects high.
    if (ua > 0xFFFFFFFFull) {
        REQUIRE(a1 != legacy_low32_decimal(ua));
    }
    if (ub > 0xFFFFFFFFull) {
        REQUIRE(b != legacy_low32_decimal(ub));
    }
    REQUIRE(ua < (1ull << 48));
    REQUIRE(ub < (1ull << 48));
#endif
}

TEST_CASE("to_pointer_rejects_immediate_values")
{
    init_output_buf();

    REQUIRE(ves_interpret("to-pointer-num", "var p = (1).to_pointer()") ==
            VES_INTERPRET_RUNTIME_ERROR);
    REQUIRE(ves_interpret("to-pointer-bool", "var p = true.to_pointer()") ==
            VES_INTERPRET_RUNTIME_ERROR);
    REQUIRE(ves_interpret("to-pointer-null", "var p = null.to_pointer()") ==
            VES_INTERPRET_RUNTIME_ERROR);

    // A rejection must not poison the next ordinary heap-object call.
    REQUIRE(ves_interpret("to-pointer-recovery", R"(
class Node {}
var p = Node().to_pointer()
System.print(p)
)") == VES_INTERPRET_OK);
    const std::string output(get_output_buf());
    REQUIRE(!output.empty());
    REQUIRE(output.back() == '\n');
    REQUIRE(output.find_first_not_of("0123456789\n") == std::string::npos);
}
