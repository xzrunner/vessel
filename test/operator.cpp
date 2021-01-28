#include "utility.h"

#include <catch/catch.hpp>

#include <vessel.h>

TEST_CASE("add")
{
    init_output_buf();

    ves_interpret("test", R"(
System.print(123 + 456) // expect: 579
System.print("str" + "ing") // expect: string
)");
    REQUIRE(std::string(get_output_buf()) == R"(
579
string
)" + 1);
}

TEST_CASE("comparison")
{
    init_output_buf();

    ves_interpret("test", R"(
System.print(1 < 2)    // expect: true
System.print(2 < 2)    // expect: false
System.print(2 < 1)    // expect: false

System.print(1 <= 2)    // expect: true
System.print(2 <= 2)    // expect: true
System.print(2 <= 1)    // expect: false

System.print(1 > 2)    // expect: false
System.print(2 > 2)    // expect: false
System.print(2 > 1)    // expect: true

System.print(1 >= 2)    // expect: false
System.print(2 >= 2)    // expect: true
System.print(2 >= 1)    // expect: true

// Zero and negative zero compare the same.
System.print(0 < -0) // expect: false
System.print(-0 < 0) // expect: false
System.print(0 > -0) // expect: false
System.print(-0 > 0) // expect: false
System.print(0 <= -0) // expect: true
System.print(-0 <= 0) // expect: true
System.print(0 >= -0) // expect: true
System.print(-0 >= 0) // expect: true
)");
    REQUIRE(std::string(get_output_buf()) == R"(
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
    init_output_buf();

    ves_interpret("test", R"(
System.print(8 / 2)         // expect: 4
System.print(12.34 / 12.34)  // expect: 1
)");
    REQUIRE(std::string(get_output_buf()) == R"(
4
1
)" + 1);
}

TEST_CASE("equals")
{
    init_output_buf();

    ves_interpret("test", R"(
System.print(nil == nil) // expect: true

System.print(true == true) // expect: true
System.print(true == false) // expect: false

System.print(1 == 1) // expect: true
System.print(1 == 2) // expect: false

System.print("str" == "str") // expect: true
System.print("str" == "ing") // expect: false

System.print(nil == false) // expect: false
System.print(false == 0) // expect: false
System.print(0 == "0") // expect: false
)");
    REQUIRE(std::string(get_output_buf()) == R"(
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
    init_output_buf();

    ves_interpret("test", R"(
// Bound methods have identity equality.
class Foo {}
class Bar {}

System.print(Foo == Foo) // expect: true
System.print(Foo == Bar) // expect: false
System.print(Bar == Foo) // expect: false
System.print(Bar == Bar) // expect: true

System.print(Foo == "Foo") // expect: false
System.print(Foo == nil)   // expect: false
System.print(Foo == 123)   // expect: false
System.print(Foo == true)  // expect: false
)");
    REQUIRE(std::string(get_output_buf()) == R"(
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
    init_output_buf();

    ves_interpret("test", R"(
// Bound methods have identity equality.
class Foo {
  method() {}
}

var foo = Foo()
var fooMethod = foo.method

// Same bound method.
System.print(fooMethod == fooMethod) // expect: true

// Different closurizations.
System.print(foo.method == foo.method) // expect: false
)");
    REQUIRE(std::string(get_output_buf()) == R"(
true
false
)" + 1);
}

TEST_CASE("multiply")
{
    init_output_buf();

    ves_interpret("test", R"(
System.print(5 * 3) // expect: 15
System.print(12.34 * 0.3) // expect: 3.702
)");
    REQUIRE(std::string(get_output_buf()) == R"(
15
3.702
)" + 1);
}

TEST_CASE("negate")
{
    init_output_buf();

    ves_interpret("test", R"(
System.print(-(3)) // expect: -3
System.print(--(3)) // expect: 3
System.print(---(3)) // expect: -3
)");
    REQUIRE(std::string(get_output_buf()) == R"(
-3
3
-3
)" + 1);
}

TEST_CASE("not")
{
    init_output_buf();

    ves_interpret("test", R"(
System.print(!true)     // expect: false
System.print(!false)    // expect: true
System.print(!!true)    // expect: true

System.print(!123)      // expect: false
System.print(!0)        // expect: false

System.print(!nil)     // expect: true

System.print(!"")       // expect: false

fun foo() {}
System.print(!foo)      // expect: false
)");
    REQUIRE(std::string(get_output_buf()) == R"(
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
    init_output_buf();

    ves_interpret("test", R"(
class Bar {}
System.print(!Bar)      // expect: false
System.print(!Bar())    // expect: false
)");
    REQUIRE(std::string(get_output_buf()) == R"(
false
false
)" + 1);
}

TEST_CASE("not_equals")
{
    init_output_buf();

    ves_interpret("test", R"(
System.print(nil != nil) // expect: false

System.print(true != true) // expect: false
System.print(true != false) // expect: true

System.print(1 != 1) // expect: false
System.print(1 != 2) // expect: true

System.print("str" != "str") // expect: false
System.print("str" != "ing") // expect: true

System.print(nil != false) // expect: true
System.print(false != 0) // expect: true
System.print(0 != "0") // expect: true
)");
    REQUIRE(std::string(get_output_buf()) == R"(
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
    init_output_buf();

    ves_interpret("test", R"(
System.print(4 - 3) // expect: 1
System.print(1.2 - 1.2) // expect: 0
)");
    REQUIRE(std::string(get_output_buf()) == R"(
1
0
)" + 1);
}