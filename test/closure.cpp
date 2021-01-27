#include <catch/catch.hpp>

#include <vessel.h>

TEST_CASE("assign_to_closure")
{
    ves_str_buf_clear();

    ves_interpret("test", R"(
var f
var g

{
  var local = "local"
  fun f_() {
    print local
    local = "after f"
    print local
  }
  f = f_

  fun g_() {
    print local
    local = "after g"
    print local
  }
  g = g_
}

f()
// expect: local
// expect: after f

g()
// expect: after f
// expect: after g
)");
    REQUIRE(std::string(ves_get_str_buf()) == R"(
local
after f
after f
after g
)" + 1);
}

TEST_CASE("assign_to_shadowed_later")
{
    ves_str_buf_clear();

    ves_interpret("test", R"(
var a = "global"

{
  fun assign() {
    a = "assigned"
  }

  var a = "inner"
  assign()
  print a // expect: inner
}

print a // expect: assigned
)");
    REQUIRE(std::string(ves_get_str_buf()) == R"(
inner
assigned
)" + 1);
}

TEST_CASE("close_over_function_parameter")
{
    ves_str_buf_clear();

    ves_interpret("test", R"(
var f

fun foo(param) {
  fun f_() {
    print param
  }
  f = f_
}
foo("param")

f() // expect: param
)");
    REQUIRE(std::string(ves_get_str_buf()) == R"(
param
)" + 1);
}

TEST_CASE("close_over_later_variable")
{
    ves_str_buf_clear();

    ves_interpret("test", R"(
// This is a regression test. There was a bug where if an upvalue for an
// earlier local (here "a") was captured *after* a later one ("b"), then it
// would crash because it walked to the end of the upvalue list (correct), but
// then didn't handle not finding the variable.

fun f() {
  var a = "a"
  var b = "b"
  fun g() {
    print b // expect: b
    print a // expect: a
  }
  g()
}
f()
)");
    REQUIRE(std::string(ves_get_str_buf()) == R"(
b
a
)" + 1);
}

TEST_CASE("close_over_method_parameter")
{
    ves_str_buf_clear();

    ves_interpret("test", R"(
var f

class Foo {
  method(param) {
    fun f_() {
      print param
    }
    f = f_
  }
}

Foo().method("param")
f() // expect: param
)");
    REQUIRE(std::string(ves_get_str_buf()) == R"(
param
)" + 1);
}

TEST_CASE("closed_closure_in_function")
{
    ves_str_buf_clear();

    ves_interpret("test", R"(
var f

{
  var local = "local"
  fun f_() {
    print local
  }
  f = f_
}

f() // expect: local
)");
    REQUIRE(std::string(ves_get_str_buf()) == R"(
local
)" + 1);
}

TEST_CASE("nested_closure")
{
    ves_str_buf_clear();

    ves_interpret("test", R"(
var f

fun f1() {
  var a = "a"
  fun f2() {
    var b = "b"
    fun f3() {
      var c = "c"
      fun f4() {
        print a
        print b
        print c
      }
      f = f4
    }
    f3()
  }
  f2()
}
f1()

f()
// expect: a
// expect: b
// expect: c
)");
    REQUIRE(std::string(ves_get_str_buf()) == R"(
a
b
c
)" + 1);
}

TEST_CASE("open_closure_in_function")
{
    ves_str_buf_clear();

    ves_interpret("test", R"(
{
  var local = "local"
  fun f() {
    print local // expect: local
  }
  f()
}
)");
    REQUIRE(std::string(ves_get_str_buf()) == R"(
local
)" + 1);
}

TEST_CASE("reference_closure_multiple_times")
{
    ves_str_buf_clear();

    ves_interpret("test", R"(
var f

{
  var a = "a"
  fun f_() {
    print a
    print a
  }
  f = f_
}

f()
// expect: a
// expect: a
)");
    REQUIRE(std::string(ves_get_str_buf()) == R"(
a
a
)" + 1);
}

TEST_CASE("reuse_closure_slot")
{
    ves_str_buf_clear();

    ves_interpret("test", R"(
{
  var f

  {
    var a = "a"
    fun f_() { print a }
    f = f_
  }

  {
    // Since a is out of scope, the local slot will be reused by b. Make sure
    // that f still closes over a.
    var b = "b"
    f() // expect: a
  }
}
)");
    REQUIRE(std::string(ves_get_str_buf()) == R"(
a
)" + 1);
}

TEST_CASE("shadow_closure_with_local")
{
    ves_str_buf_clear();

    ves_interpret("test", R"(
{
  var foo = "closure"
  fun f() {
    {
      print foo // expect: closure
      var foo = "shadow"
      print foo // expect: shadow
    }
    print foo // expect: closure
  }
  f()
}
)");
    REQUIRE(std::string(ves_get_str_buf()) == R"(
closure
shadow
closure
)" + 1);
}

TEST_CASE("unused_closure")
{
    ves_str_buf_clear();

    ves_interpret("test", R"(
// This is a regression test. There was a bug where the VM would try to close
// an upvalue even if the upvalue was never created because the codepath for
// the closure was not executed.

{
  var a = "a"
  if (false) {
    fun foo() { a }
  }
}

// If we get here, we didn't segfault when a went out of scope.
print "ok" // expect: ok
)");
    REQUIRE(std::string(ves_get_str_buf()) == R"(
ok
)" + 1);
}

TEST_CASE("unused_later_closure")
{
    ves_str_buf_clear();

    ves_interpret("test", R"(
// This is a regression test. When closing upvalues for discarded locals, it
// wouldn't make sure it discarded the upvalue for the correct stack slot.
//
// Here we create two locals that can be closed over, but only the first one
// actually is. When "b" goes out of scope, we need to make sure we don't
// prematurely close "a".
var closure

{
  var a = "a"

  {
    var b = "b"
    fun returnA() {
      return a
    }

    closure = returnA

    if (false) {
      fun returnB() {
        return b
      }
    }
  }

  print closure() // expect: a
}
)");
    REQUIRE(std::string(ves_get_str_buf()) == R"(
a
)" + 1);
}