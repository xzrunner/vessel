#include "utility.h"

#include <catch/catch.hpp>

#include <vessel.h>

TEST_CASE("assign_to_closure")
{
    init_output_buf();

    ves_interpret("test", R"(
var f
var g

{
  var local = "local"
  fun f_() {
    System.print(local)
    local = "after f"
    System.print(local)
  }
  f = f_

  fun g_() {
    System.print(local)
    local = "after g"
    System.print(local)
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
    REQUIRE(std::string(get_output_buf()) == R"(
local
after f
after f
after g
)" + 1);
}

TEST_CASE("assign_to_shadowed_later")
{
    init_output_buf();

    ves_interpret("test", R"(
var a = "global"

{
  fun assign() {
    a = "assigned"
  }

  var a = "inner"
  assign()
  System.print(a) // expect: inner
}

System.print(a) // expect: assigned
)");
    REQUIRE(std::string(get_output_buf()) == R"(
inner
assigned
)" + 1);
}

TEST_CASE("close_over_function_parameter")
{
    init_output_buf();

    ves_interpret("test", R"(
var f

fun foo(param) {
  fun f_() {
    System.print(param)
  }
  f = f_
}
foo("param")

f() // expect: param
)");
    REQUIRE(std::string(get_output_buf()) == R"(
param
)" + 1);
}

TEST_CASE("close_over_later_variable")
{
    init_output_buf();

    ves_interpret("test", R"(
// This is a regression test. There was a bug where if an upvalue for an
// earlier local (here "a") was captured *after* a later one ("b"), then it
// would crash because it walked to the end of the upvalue list (correct), but
// then didn't handle not finding the variable.

fun f() {
  var a = "a"
  var b = "b"
  fun g() {
    System.print(b) // expect: b
    System.print(a) // expect: a
  }
  g()
}
f()
)");
    REQUIRE(std::string(get_output_buf()) == R"(
b
a
)" + 1);
}

TEST_CASE("close_over_method_parameter")
{
    init_output_buf();

    ves_interpret("test", R"(
var f

class Foo {
  method(param) {
    fun f_() {
      System.print(param)
    }
    f = f_
  }
}

Foo().method("param")
f() // expect: param
)");
    REQUIRE(std::string(get_output_buf()) == R"(
param
)" + 1);
}

TEST_CASE("closed_closure_in_function")
{
    init_output_buf();

    ves_interpret("test", R"(
var f

{
  var local = "local"
  fun f_() {
    System.print(local)
  }
  f = f_
}

f() // expect: local
)");
    REQUIRE(std::string(get_output_buf()) == R"(
local
)" + 1);
}

TEST_CASE("nested_closure")
{
    init_output_buf();

    ves_interpret("test", R"(
var f

fun f1() {
  var a = "a"
  fun f2() {
    var b = "b"
    fun f3() {
      var c = "c"
      fun f4() {
        System.print(a)
        System.print(b)
        System.print(c)
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
    REQUIRE(std::string(get_output_buf()) == R"(
a
b
c
)" + 1);
}

TEST_CASE("open_closure_in_function")
{
    init_output_buf();

    ves_interpret("test", R"(
{
  var local = "local"
  fun f() {
    System.print(local) // expect: local
  }
  f()
}
)");
    REQUIRE(std::string(get_output_buf()) == R"(
local
)" + 1);
}

TEST_CASE("reference_closure_multiple_times")
{
    init_output_buf();

    ves_interpret("test", R"(
var f

{
  var a = "a"
  fun f_() {
    System.print(a)
    System.print(a)
  }
  f = f_
}

f()
// expect: a
// expect: a
)");
    REQUIRE(std::string(get_output_buf()) == R"(
a
a
)" + 1);
}

TEST_CASE("reuse_closure_slot")
{
    init_output_buf();

    ves_interpret("test", R"(
{
  var f

  {
    var a = "a"
    fun f_() { System.print(a) }
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
    REQUIRE(std::string(get_output_buf()) == R"(
a
)" + 1);
}

TEST_CASE("shadow_closure_with_local")
{
    init_output_buf();

    ves_interpret("test", R"(
{
  var foo = "closure"
  fun f() {
    {
      System.print(foo) // expect: closure
      var foo = "shadow"
      System.print(foo) // expect: shadow
    }
    System.print(foo) // expect: closure
  }
  f()
}
)");
    REQUIRE(std::string(get_output_buf()) == R"(
closure
shadow
closure
)" + 1);
}

TEST_CASE("unused_closure")
{
    init_output_buf();

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
System.print("ok") // expect: ok
)");
    REQUIRE(std::string(get_output_buf()) == R"(
ok
)" + 1);
}

TEST_CASE("unused_later_closure")
{
    init_output_buf();

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

  System.print(closure()) // expect: a
}
)");
    REQUIRE(std::string(get_output_buf()) == R"(
a
)" + 1);
}