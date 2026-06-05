#include "utility.h"

#include <catch2/catch_test_macros.hpp>

#include <vessel.h>

TEST_CASE("conditional_basic")
{
    init_output_buf();

    ves_interpret("test", R"(
System.print(true ? "yes" : "no")  // expect: yes
System.print(false ? "yes" : "no") // expect: no

// nil and false are falsey; everything else is truthy.
System.print(nil ? "yes" : "no")   // expect: no
System.print(0 ? "yes" : "no")     // expect: yes
System.print("" ? "yes" : "no")    // expect: yes
)");
    REQUIRE(std::string(get_output_buf()) == R"(
yes
no
no
yes
yes
)" + 1);
}

TEST_CASE("conditional_precedence")
{
    init_output_buf();

    ves_interpret("test", R"(
// The condition binds tighter than '?:'.
System.print(1 + 1 == 2 ? "a" : "b") // expect: a
System.print(2 > 3 ? "a" : "b")      // expect: b

// The branches may be full expressions.
System.print(true ? 1 + 2 : 3 + 4)   // expect: 3
System.print(false ? 1 + 2 : 3 + 4)  // expect: 7
)");
    REQUIRE(std::string(get_output_buf()) == R"(
a
b
3
7
)" + 1);
}

TEST_CASE("conditional_associativity")
{
    init_output_buf();

    ves_interpret("test", R"(
// Right associative: a ? b : c ? d : e == a ? b : (c ? d : e)
System.print(false ? "a" : true ? "b" : "c")  // expect: b
System.print(false ? "a" : false ? "b" : "c") // expect: c
System.print(true ? "a" : true ? "b" : "c")   // expect: a

// Nested in the then branch.
System.print(true ? true ? "x" : "y" : "z")   // expect: x
System.print(true ? false ? "x" : "y" : "z")  // expect: y
)");
    REQUIRE(std::string(get_output_buf()) == R"(
b
c
a
x
y
)" + 1);
}

TEST_CASE("conditional_statements")
{
    init_output_buf();

    ves_interpret("test", R"(
// As a variable initializer.
var a = true ? 10 : 20
System.print(a) // expect: 10

var b = 5
var c = b > 3 ? "big" : "small"
System.print(c) // expect: big

// As an assignment right-hand side.
var d = 0
d = false ? 1 : 2
System.print(d) // expect: 2

// Spanning multiple lines.
var v = false ?
    "first" :
    "second"
System.print(v) // expect: second
)");
    REQUIRE(std::string(get_output_buf()) == R"(
10
big
2
second
)" + 1);
}

TEST_CASE("conditional_only_one_branch")
{
    init_output_buf();

    ves_interpret("test", R"(
// Only the selected branch is evaluated; the other has no side effect.
var taken = nil
var skipped = nil
var r = true ? (taken = 1) : (skipped = 2)
System.print(r)       // expect: 1
System.print(taken)   // expect: 1
System.print(skipped) // expect: nil

// The other direction.
var t2 = nil
var s2 = nil
var r2 = false ? (t2 = 10) : (s2 = 20)
System.print(r2)      // expect: 20
System.print(t2)      // expect: nil
System.print(s2)      // expect: 20
)");
    REQUIRE(std::string(get_output_buf()) == R"(
1
1
nil
20
nil
20
)" + 1);
}

TEST_CASE("conditional_in_collections")
{
    init_output_buf();

    ves_interpret("test", R"(
// As list elements.
var xs = [true ? 1 : 2, false ? 3 : 4]
System.print(xs[0]) // expect: 1
System.print(xs[1]) // expect: 4

// As a map value.
var m = { "k": 3 > 2 ? "big" : "small" }
System.print(m["k"]) // expect: big

// As a call argument.
System.print(true ? "arg-then" : "arg-else") // expect: arg-then
)");
    REQUIRE(std::string(get_output_buf()) == R"(
1
4
big
arg-then
)" + 1);
}
