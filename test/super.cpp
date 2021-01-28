#include "utility.h"

#include <catch/catch.hpp>

#include <vessel.h>

TEST_CASE("bound_method")
{
    init_output_buf();

    ves_interpret("test", R"foo(
class A {
  method(arg) {
    System.print("A.method(" + arg + ")")
  }
}

class B is A {
  getClosure() {
    return super.method
  }

  method(arg) {
    System.print("B.method(" + arg + ")")
  }
}


var closure = B().getClosure()
closure("arg") // expect: A.method(arg)
)foo");
    REQUIRE(std::string(get_output_buf()) == R"(
A.method(arg)
)" + 1);
}

TEST_CASE("call_other_method")
{
    init_output_buf();

    ves_interpret("test", R"foo(
class Base {
  foo() {
    System.print("Base.foo()")
}
}

class Derived is Base{
  bar() {
    System.print("Derived.bar()")
    super.foo()
  }
}

Derived().bar()
// expect: Derived.bar()
// expect: Base.foo()
)foo");
    REQUIRE(std::string(get_output_buf()) == R"(
Derived.bar()
Base.foo()
)" + 1);
}

TEST_CASE("call_same_method")
{
    init_output_buf();

    ves_interpret("test", R"foo(
class Base {
  foo() {
    System.print("Base.foo()")
  }
}

class Derived is Base {
  foo() {
    System.print("Derived.foo()")
    super.foo()
  }
}

Derived().foo()
// expect: Derived.foo()
// expect: Base.foo()
)foo");
    REQUIRE(std::string(get_output_buf()) == R"(
Derived.foo()
Base.foo()
)" + 1);
}

TEST_CASE("closure")
{
    init_output_buf();

    ves_interpret("test", R"(
class Base {
  toString() { return "Base" }
}

class Derived is Base {
  getClosure() {
    fun closure() {
      return super.toString()
    }
    return closure
  }

  toString() { return "Derived" }
}

var closure = Derived().getClosure()
System.print(closure()) // expect: Base
)");
    REQUIRE(std::string(get_output_buf()) == R"(
Base
)" + 1);
}

TEST_CASE("constructor")
{
    init_output_buf();

    ves_interpret("test", R"foo(
class Base {
  init(a, b) {
    System.print("Base.init(" + a + ", " + b + ")")
}
}

class Derived is Base{
  init() {
    System.print("Derived.init()")
    super.init("a", "b")
  }
}

Derived()
// expect: Derived.init()
// expect: Base.init(a, b)
)foo");
    REQUIRE(std::string(get_output_buf()) == R"(
Derived.init()
Base.init(a, b)
)" + 1);
}

TEST_CASE("indirectly_inherited")
{
    init_output_buf();

    ves_interpret("test", R"foo(
class A {
  foo() {
    System.print("A.foo()")
  }
}

class B is A {}

class C is B {
  foo() {
    System.print("C.foo()")
    super.foo()
  }
}

C().foo()
// expect: C.foo()
// expect: A.foo()
)foo");
    REQUIRE(std::string(get_output_buf()) == R"(
C.foo()
A.foo()
)" + 1);
}

TEST_CASE("reassign_superclass")
{
    init_output_buf();

    ves_interpret("test", R"foo(
class Base {
  method() {
    System.print("Base.method()")
  }
}

class Derived is Base {
  method() {
    super.method()
  }
}

class OtherBase {
  method() {
    System.print("OtherBase.method()")
  }
}

var derived = Derived()
derived.method() // expect: Base.method()
Base = OtherBase
derived.method() // expect: Base.method()
)foo");
    REQUIRE(std::string(get_output_buf()) == R"(
Base.method()
Base.method()
)" + 1);
}

TEST_CASE("super_in_closure_in_inherited_method")
{
    init_output_buf();

    ves_interpret("test", R"foo(
class A {
  say() {
    System.print("A")
  }
}

class B is A {
  getClosure() {
    fun closure() {
      super.say()
    }
    return closure
  }

  say() {
    System.print("B")
  }
}

class C is B {
  say() {
    System.print("C")
  }
}

C().getClosure()() // expect: A
)foo");
    REQUIRE(std::string(get_output_buf()) == R"(
A
)" + 1);
}

TEST_CASE("super_in_inherited_method")
{
    init_output_buf();

    ves_interpret("test", R"foo(
class A {
  say() {
    System.print("A")
  }
}

class B is A {
  test() {
    super.say()
  }

  say() {
    System.print("B")
  }
}

class C is B {
  say() {
    System.print("C")
  }
}

C().test() // expect: A
)foo");
    REQUIRE(std::string(get_output_buf()) == R"(
A
)" + 1);
}

TEST_CASE("this_in_superclass_method")
{
    init_output_buf();

    ves_interpret("test", R"foo(
class Base {
  init(a) {
    this.a = a
  }
}

class Derived is Base {
  init(a, b) {
    super.init(a)
    this.b = b
  }
}

var derived = Derived("a", "b")
System.print(derived.a) // expect: a
System.print(derived.b) // expect: b
)foo");
    REQUIRE(std::string(get_output_buf()) == R"(
a
b
)" + 1);
}