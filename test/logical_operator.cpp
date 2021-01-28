#include "utility.h"

#include <catch/catch.hpp>

#include <vessel.h>

TEST_CASE("and")
{
    init_output_buf();

    ves_interpret("test", R"(
// Note: These tests implicitly depend on ints being truthy.

// Return the first non-true argument.
System.print(false and 1) // expect: false
System.print(true and 1) // expect: 1
System.print(1 and 2 and false) // expect: false

// Return the last argument if all are true.
System.print(1 and true) // expect: true
System.print(1 and 2 and 3) // expect: 3

// Short-circuit at the first false argument.
var a = "before"
var b = "before"
(a = true) and
    (b = false) and
    (a = "bad")
System.print(a) // expect: true
System.print(b) // expect: false
)");
    REQUIRE(std::string(get_output_buf()) == R"(
false
1
false
true
3
true
false
)" + 1);
}

TEST_CASE("and_truth")
{
    init_output_buf();

    ves_interpret("test", R"(
// False and nil are false.
System.print(false and "bad") // expect: false
System.print(nil and "bad") // expect: nil

// Everything else is true.
System.print(true and "ok") // expect: ok
System.print(0 and "ok") // expect: ok
System.print("" and "ok") // expect: ok
)");
    REQUIRE(std::string(get_output_buf()) == R"(
false
nil
ok
ok
ok
)" + 1);
}

TEST_CASE("or")
{
    init_output_buf();

    ves_interpret("test", R"(
// Note: These tests implicitly depend on ints being truthy.

// Return the first true argument.
System.print(1 or true) // expect: 1
System.print(false or 1) // expect: 1
System.print(false or false or true) // expect: true

// Return the last argument if all are false.
System.print(false or false) // expect: false
System.print(false or false or false) // expect: false

// Short-circuit at the first true argument.
var a = "before"
var b = "before"
(a = false) or
    (b = true) or
    (a = "bad")
System.print(a) // expect: false
System.print(b) // expect: true
)");
    REQUIRE(std::string(get_output_buf()) == R"(
1
1
true
false
false
false
true
)" + 1);
}

TEST_CASE("or_truth")
{
    init_output_buf();

    ves_interpret("test", R"(
// False and nil are false.
System.print(false or "ok") // expect: ok
System.print(nil or "ok") // expect: ok

// Everything else is true.
System.print(true or "ok") // expect: true
System.print(0 or "ok") // expect: 0
System.print("s" or "ok") // expect: s
)");
    REQUIRE(std::string(get_output_buf()) == R"(
ok
ok
true
0
s
)" + 1);
}