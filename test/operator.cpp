#include <catch/catch.hpp>

#include <vessel.h>

TEST_CASE("add")
{
    vessel_str_buf_clear();

    vessel_interpret("test", R"(
print 123 + 456 // expect: 579
print "str" + "ing" // expect: string
)");
    REQUIRE(std::string(vessel_get_str_buf()) == R"(
579
string
)" + 1);
}

TEST_CASE("comparison")
{
    vessel_str_buf_clear();

    vessel_interpret("test", R"(
print 1 < 2    // expect: true
print 2 < 2    // expect: false
print 2 < 1    // expect: false

print 1 <= 2    // expect: true
print 2 <= 2    // expect: true
print 2 <= 1    // expect: false

print 1 > 2    // expect: false
print 2 > 2    // expect: false
print 2 > 1    // expect: true

print 1 >= 2    // expect: false
print 2 >= 2    // expect: true
print 2 >= 1    // expect: true

// Zero and negative zero compare the same.
print 0 < -0 // expect: false
print -0 < 0 // expect: false
print 0 > -0 // expect: false
print -0 > 0 // expect: false
print 0 <= -0 // expect: true
print -0 <= 0 // expect: true
print 0 >= -0 // expect: true
print -0 >= 0 // expect: true
)");
    REQUIRE(std::string(vessel_get_str_buf()) == R"(
true
false
false
true
true
false
false
false
true
false
true
true
false
false
false
false
true
true
true
true
)" + 1);
}

TEST_CASE("divide")
{
    vessel_str_buf_clear();

    vessel_interpret("test", R"(
print 8 / 2         // expect: 4
print 12.34 / 12.34  // expect: 1
)");
    REQUIRE(std::string(vessel_get_str_buf()) == R"(
4
1
)" + 1);
}

TEST_CASE("equals")
{
    vessel_str_buf_clear();

    vessel_interpret("test", R"(
print nil == nil // expect: true

print true == true // expect: true
print true == false // expect: false

print 1 == 1 // expect: true
print 1 == 2 // expect: false

print "str" == "str" // expect: true
print "str" == "ing" // expect: false

print nil == false // expect: false
print false == 0 // expect: false
print 0 == "0" // expect: false
)");
    REQUIRE(std::string(vessel_get_str_buf()) == R"(
true
true
false
true
false
true
false
false
false
false
)" + 1);
}

TEST_CASE("equals_class")
{
    vessel_str_buf_clear();

    vessel_interpret("test", R"(
// Bound methods have identity equality.
class Foo {}
class Bar {}

print Foo == Foo // expect: true
print Foo == Bar // expect: false
print Bar == Foo // expect: false
print Bar == Bar // expect: true

print Foo == "Foo" // expect: false
print Foo == nil   // expect: false
print Foo == 123   // expect: false
print Foo == true  // expect: false
)");
    REQUIRE(std::string(vessel_get_str_buf()) == R"(
true
false
false
true
false
false
false
false
)" + 1);
}

TEST_CASE("equals_method")
{
    vessel_str_buf_clear();

    vessel_interpret("test", R"(
// Bound methods have identity equality.
class Foo {
  method() {}
}

var foo = Foo()
var fooMethod = foo.method

// Same bound method.
print fooMethod == fooMethod // expect: true

// Different closurizations.
print foo.method == foo.method // expect: false
)");
    REQUIRE(std::string(vessel_get_str_buf()) == R"(
true
false
)" + 1);
}

TEST_CASE("multiply")
{
    vessel_str_buf_clear();

    vessel_interpret("test", R"(
print 5 * 3 // expect: 15
print 12.34 * 0.3 // expect: 3.702
)");
    REQUIRE(std::string(vessel_get_str_buf()) == R"(
15
3.702
)" + 1);
}

TEST_CASE("negate")
{
    vessel_str_buf_clear();

    vessel_interpret("test", R"(
print -(3) // expect: -3
print --(3) // expect: 3
print ---(3) // expect: -3
)");
    REQUIRE(std::string(vessel_get_str_buf()) == R"(
-3
3
-3
)" + 1);
}

TEST_CASE("not")
{
    vessel_str_buf_clear();

    vessel_interpret("test", R"(
print !true     // expect: false
print !false    // expect: true
print !!true    // expect: true

print !123      // expect: false
print !0        // expect: false

print !nil     // expect: true

print !""       // expect: false

fun foo() {}
print !foo      // expect: false
)");
    REQUIRE(std::string(vessel_get_str_buf()) == R"(
false
true
true
false
false
true
false
false
)" + 1);
}

TEST_CASE("not_class")
{
    vessel_str_buf_clear();

    vessel_interpret("test", R"(
class Bar {}
print !Bar      // expect: false
print !Bar()    // expect: false
)");
    REQUIRE(std::string(vessel_get_str_buf()) == R"(
false
false
)" + 1);
}

TEST_CASE("not_equals")
{
    vessel_str_buf_clear();

    vessel_interpret("test", R"(
print nil != nil // expect: false

print true != true // expect: false
print true != false // expect: true

print 1 != 1 // expect: false
print 1 != 2 // expect: true

print "str" != "str" // expect: false
print "str" != "ing" // expect: true

print nil != false // expect: true
print false != 0 // expect: true
print 0 != "0" // expect: true
)");
    REQUIRE(std::string(vessel_get_str_buf()) == R"(
false
false
true
false
true
false
true
true
true
true
)" + 1);
}

TEST_CASE("subtract")
{
    vessel_str_buf_clear();

    vessel_interpret("test", R"(
print 4 - 3 // expect: 1
print 1.2 - 1.2 // expect: 0
)");
    REQUIRE(std::string(vessel_get_str_buf()) == R"(
1
0
)" + 1);
}