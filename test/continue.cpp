#include "utility.h"

#include <catch2/catch_test_macros.hpp>

#include <vessel.h>

TEST_CASE("continue_in_while")
{
    init_output_buf();

    ves_interpret("test", R"(
var i = 0
while (i < 5) {
  i = i + 1
  if (i == 3) continue
  System.print(i)
}
)");
    REQUIRE(std::string(get_output_buf()) == R"(
1
2
4
5
)" + 1);
}

TEST_CASE("continue_in_for")
{
    init_output_buf();

    ves_interpret("test", R"(
for (var i = 0; i < 5; i = i + 1) {
  if (i == 2) continue
  if (i == 4) continue
  System.print(i)
}
)");
    REQUIRE(std::string(get_output_buf()) == R"(
0
1
3
)" + 1);
}

TEST_CASE("continue_in_for_in")
{
    init_output_buf();

    ves_interpret("test", R"(
for (var i in 0..5) {
  if (i == 1) continue
  if (i == 3) continue
  System.print(i)
}
)");
    REQUIRE(std::string(get_output_buf()) == R"(
0
2
4
)" + 1);
}

TEST_CASE("continue_in_nested_loops")
{
    init_output_buf();

    ves_interpret("test", R"(
for (var i = 0; i < 3; i = i + 1) {
  for (var j = 0; j < 3; j = j + 1) {
    if (j == 1) continue
    System.print(i * 10 + j)
  }
}
)");
    REQUIRE(std::string(get_output_buf()) == R"(
0
2
10
12
20
22
)" + 1);
}

TEST_CASE("break_in_while")
{
    init_output_buf();

    ves_interpret("test", R"(
var i = 0
while (i < 10) {
  if (i == 3) break
  System.print(i)
  i = i + 1
}
)");
    REQUIRE(std::string(get_output_buf()) == R"(
0
1
2
)" + 1);
}

TEST_CASE("break_in_for")
{
    init_output_buf();

    ves_interpret("test", R"(
for (var i = 0; i < 10; i = i + 1) {
  if (i == 3) break
  System.print(i)
}
)");
    REQUIRE(std::string(get_output_buf()) == R"(
0
1
2
)" + 1);
}

TEST_CASE("break_in_for_in")
{
    init_output_buf();

    ves_interpret("test", R"(
for (var i in 0..10) {
  if (i == 3) break
  System.print(i)
}
)");
    REQUIRE(std::string(get_output_buf()) == R"(
0
1
2
)" + 1);
}

TEST_CASE("break_in_nested_loops")
{
    init_output_buf();

    ves_interpret("test", R"(
for (var i = 0; i < 3; i = i + 1) {
  for (var j = 0; j < 10; j = j + 1) {
    if (j == 2) break
    System.print(i * 10 + j)
  }
}
)");
    REQUIRE(std::string(get_output_buf()) == R"(
0
1
10
11
20
21
)" + 1);
}

TEST_CASE("break_and_continue_combined")
{
    init_output_buf();

    ves_interpret("test", R"(
for (var i = 0; i < 10; i = i + 1) {
  if (i == 2) continue
  if (i == 5) break
  System.print(i)
}
)");
    REQUIRE(std::string(get_output_buf()) == R"(
0
1
3
4
)" + 1);
}

TEST_CASE("continue_with_local_vars")
{
    init_output_buf();

    ves_interpret("test", R"(
for (var i = 0; i < 4; i = i + 1) {
  var x = i * 10
  if (i == 1) continue
  System.print(x)
}
)");
    REQUIRE(std::string(get_output_buf()) == R"(
0
20
30
)" + 1);
}
