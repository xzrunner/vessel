#include <catch/catch.hpp>

#include <vessel.h>

TEST_CASE("list_add")
{
    ves_str_buf_clear();

    interpret("test", R"(
var a = [1]
a.add(2)
print(a) // expect: [1, 2]
a.add(3)
print(a) // expect: [1, 2, 3]

// Returns added element.
print(a.add(4)) // expect: 4
)");
    REQUIRE(std::string(ves_get_str_buf()) == R"(
[1, 2]
[1, 2, 3]
4
)" + 1);
}

TEST_CASE("list_clear")
{
    ves_str_buf_clear();

    interpret("test", R"(
var a = [1, 2, 3]
a.clear()
print(a)       // expect: []
print(a.count) // expect: 0

// Returns null.
print([1, 2].clear()) // expect: nil
)");
    REQUIRE(std::string(ves_get_str_buf()) == R"(
[]
0
nil
)" + 1);
}

TEST_CASE("list_remove_at")
{
    ves_str_buf_clear();

    interpret("test", R"(
var a = [1, 2, 3]
a.removeAt(0)
print(a) // expect: [2, 3]

var b = [1, 2, 3]
b.removeAt(1)
print(b) // expect: [1, 3]

var c = [1, 2, 3]
c.removeAt(2)
print(c) // expect: [1, 2]

// Index backwards from end.
var d = [1, 2, 3]
d.removeAt(-3)
print(d) // expect: [2, 3]

var e = [1, 2, 3]
e.removeAt(-2)
print(e) // expect: [1, 3]

var f = [1, 2, 3]
f.removeAt(-1)
print(f) // expect: [1, 2]

// Return the removed value.
print([3, 4, 5].removeAt(1)) // expect: 4
)");
    REQUIRE(std::string(ves_get_str_buf()) == R"(
[2, 3]
[1, 3]
[1, 2]
[2, 3]
[1, 3]
[1, 2]
4
)" + 1);
}

TEST_CASE("list_subscript")
{
    ves_str_buf_clear();

    interpret("test", R"(
// Returns elements.
var list = ["a", "b", "c", "d"]
print(list[0]) // expect: a
print(list[1]) // expect: b
print(list[2]) // expect: c
print(list[3]) // expect: d

// Allows indexing backwards from the end.
print(list[-4]) // expect: a
print(list[-3]) // expect: b
print(list[-2]) // expect: c
print(list[-1]) // expect: d
)");
    REQUIRE(std::string(ves_get_str_buf()) == R"(
a
b
c
d
a
b
c
d
)" + 1);
}


TEST_CASE("list_subscript_setter")
{
    ves_str_buf_clear();

    interpret("test", R"(
// Basic assignment.
{
  var list = [1, 2, 3]
  list[0] = 5
  list[1] = 6
  list[2] = 7
  print(list) // expect: [5, 6, 7]
}

// Returns right-hand side.
{
  var list = [1, 2, 3]
  print(list[1] = 5) // expect: 5
}

// Negative indices.
{
  var list = [1, 2, 3]
  list[-1] = 5
  list[-2] = 6
  list[-3] = 7
  print(list) // expect: [7, 6, 5]
}
)");
    REQUIRE(std::string(ves_get_str_buf()) == R"(
[5, 6, 7]
5
[7, 6, 5]
)" + 1);
}
