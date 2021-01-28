#include "utility.h"

#include <catch/catch.hpp>

#include <vessel.h>

TEST_CASE("empty_body")
{
    init_output_buf();

    ves_interpret("test", R"(
fun f() {}
System.print(f()) // expect: nil
)");
    REQUIRE(std::string(get_output_buf()) == R"(
nil
)" + 1);
}

TEST_CASE("local_recursion")
{
    init_output_buf();

    ves_interpret("test", R"(
{
  fun fib(n) {
    if (n < 2) return n
    return fib(n - 1) + fib(n - 2)
  }

  System.print(fib(8)) // expect: 21
}
)");
    REQUIRE(std::string(get_output_buf()) == R"(
21
)" + 1);
}

TEST_CASE("mutual_recursion")
{
    init_output_buf();

    ves_interpret("test", R"(
fun isEven(n) {
  if (n == 0) return true
  return isOdd(n - 1)
}

fun isOdd(n) {
  if (n == 0) return false
  return isEven(n - 1)
}

System.print(isEven(4)) // expect: true
System.print(isOdd(3)) // expect: true
)");
    REQUIRE(std::string(get_output_buf()) == R"(
true
true
)" + 1);
}

TEST_CASE("parameters")
{
    init_output_buf();

    ves_interpret("test", R"(
fun f0() { return 0 }
System.print(f0()) // expect: 0

fun f1(a) { return a }
System.print(f1(1)) // expect: 1

fun f2(a, b) { return a + b }
System.print(f2(1, 2)) // expect: 3

fun f3(a, b, c) { return a + b + c }
System.print(f3(1, 2, 3)) // expect: 6

fun f4(a, b, c, d) { return a + b + c + d }
System.print(f4(1, 2, 3, 4)) // expect: 10

fun f5(a, b, c, d, e) { return a + b + c + d + e }
System.print(f5(1, 2, 3, 4, 5)) // expect: 15

fun f6(a, b, c, d, e, f) { return a + b + c + d + e + f }
System.print(f6(1, 2, 3, 4, 5, 6)) // expect: 21

fun f7(a, b, c, d, e, f, g) { return a + b + c + d + e + f + g }
System.print(f7(1, 2, 3, 4, 5, 6, 7)) // expect: 28

fun f8(a, b, c, d, e, f, g, h) { return a + b + c + d + e + f + g + h }
System.print(f8(1, 2, 3, 4, 5, 6, 7, 8)) // expect: 36
)");
    REQUIRE(std::string(get_output_buf()) == R"(
0
1
3
6
10
15
21
28
36
)" + 1);
}

TEST_CASE("System.print")
{
    init_output_buf();

    ves_interpret("test", R"(
fun foo() {}
System.print(foo) // expect: <fn foo>

System.print(clock) // expect: <native fn>
)");
    REQUIRE(std::string(get_output_buf()) == R"(
<fn foo>
<native fn>
)" + 1);
}

TEST_CASE("recursion")
{
    init_output_buf();

    ves_interpret("test", R"(
fun fib(n) {
  if (n < 2) return n
  return fib(n - 1) + fib(n - 2)
}

System.print(fib(8)) // expect: 21
)");
    REQUIRE(std::string(get_output_buf()) == R"(
21
)" + 1);
}