#include <catch/catch.hpp>

#include <vessel.h>

TEST_CASE("closure_in_body")
{
    ves_str_buf_clear();

    ves_interpret("test", R"(
var f1
var f2
var f3

for (var i = 1; i < 4; i = i + 1) {
  var j = i
  fun f() {
    print i
    print j
  }

  if (j == 1) f1 = f
  else if (j == 2) f2 = f
  else f3 = f
}

f1() // expect: 4
     // expect: 1
f2() // expect: 4
     // expect: 2
f3() // expect: 4
     // expect: 3
)");
    REQUIRE(std::string(ves_get_str_buf()) == R"(
4
1
4
2
4
3
)" + 1);
}

TEST_CASE("return_closure")
{
    ves_str_buf_clear();

    ves_interpret("test", R"(
fun f() {
  for (;;) {
    var i = "i"
    fun g() { print i }
    return g
  }
}

var h = f()
h() // expect: i
)");
    REQUIRE(std::string(ves_get_str_buf()) == R"(
i
)" + 1);
}

TEST_CASE("return_inside")
{
    ves_str_buf_clear();

    ves_interpret("test", R"(
fun f() {
  for (;;) {
    var i = "i"
    return i
  }
}

print f()
// expect: i
)");
    REQUIRE(std::string(ves_get_str_buf()) == R"(
i
)" + 1);
}

TEST_CASE("for-scope")
{
    ves_str_buf_clear();

    ves_interpret("test", R"(
{
  var i = "before"

  // New variable is in inner scope.
  for (var i = 0; i < 1; i = i + 1) {
    print i // expect: 0

    // Loop body is in second inner scope.
    var i = -1
    print i // expect: -1
  }
}

{
  // New variable shadows outer variable.
  for (var i = 0; i > 0; i = i + 1) {}

  // Goes out of scope after loop.
  var i = "after"
  print i // expect: after

  // Can reuse an existing variable.
  for (i = 0; i < 1; i = i + 1) {
    print i // expect: 0
  }
}
)");
    REQUIRE(std::string(ves_get_str_buf()) == R"(
0
-1
after
0
)" + 1);
}

TEST_CASE("syntax")
{
    ves_str_buf_clear();

    ves_interpret("test", R"(
// Single-expression body.
for (var c = 0; c < 3;) print c = c + 1
// expect: 1
// expect: 2
// expect: 3

// Block body.
for (var a = 0; a < 3; a = a + 1) {
  print a
}
// expect: 0
// expect: 1
// expect: 2

// No clauses.
fun foo() {
  for (;;) return "done"
}
print foo() // expect: done

// No variable.
var i = 0
for (; i < 2; i = i + 1) print i
// expect: 0
// expect: 1

// No condition.
fun bar() {
  for (var i = 0;; i = i + 1) {
    print i
    if (i >= 2) return
  }
}
bar()
// expect: 0
// expect: 1
// expect: 2

// No increment.
for (var i = 0; i < 2;) {
  print i
  i = i + 1
}
// expect: 0
// expect: 1

// Statement bodies.
for (; false;) if (true) 1 else 2
for (; false;) while (true) 1
for (; false;) for (;;) 1
)");
    REQUIRE(std::string(ves_get_str_buf()) == R"(
1
2
3
0
1
2
done
0
1
0
1
2
0
1
)" + 1);
}

TEST_CASE("in_range")
{
    ves_str_buf_clear();

    ves_interpret("test", R"(
for (var i in 0..3) {
    print(i)
}
)");
    REQUIRE(std::string(ves_get_str_buf()) == R"(
0
1
2
)" + 1);
}

TEST_CASE("in_range_equal")
{
    ves_str_buf_clear();

    ves_interpret("test", R"(
for (var i in 0..=3) {
    print(i)
}
)");
    REQUIRE(std::string(ves_get_str_buf()) == R"(
0
1
2
3
)" + 1);
}

TEST_CASE("in_list")
{
    ves_str_buf_clear();

    ves_interpret("test", R"(
var list = [0,2,3]
for (var i in list) {
    print(i)
}
)");
    REQUIRE(std::string(ves_get_str_buf()) == R"(
0
2
3
)" + 1);
}

TEST_CASE("in_map")
{
    ves_str_buf_clear();

    ves_interpret("test", R"(
var map = {
  "one": 1,
  "two": 2,
  "three": 3
}
for (var i in map) {
    print(i)
}
)");
    REQUIRE(std::string(ves_get_str_buf()) == R"(
two
three
one
)" + 1);
}