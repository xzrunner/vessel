#include <catch/catch.hpp>

#include <vessel.h>

TEST_CASE("bound_method")
{
    vessel_str_buf_clear();

    vessel_interpret("test", R"foo(
class A {
  method(arg) {
    print "A.method(" + arg + ")"
  }
}

class B is A {
  getClosure() {
    return super.method
  }

  method(arg) {
    print "B.method(" + arg + ")"
  }
}


var closure = B().getClosure()
closure("arg") // expect: A.method(arg)
)foo");
    REQUIRE(std::string(vessel_get_str_buf()) == R"(
A.method(arg)
)" + 1);
}

TEST_CASE("call_other_method")
{
    vessel_str_buf_clear();

    vessel_interpret("test", R"foo(
class Base {
  foo() {
    print "Base.foo()"
}
}

class Derived is Base{
  bar() {
    print "Derived.bar()"
    super.foo()
  }
}

Derived().bar()
// expect: Derived.bar()
// expect: Base.foo()
)foo");
    REQUIRE(std::string(vessel_get_str_buf()) == R"(
Derived.bar()
Base.foo()
)" + 1);
}

TEST_CASE("call_same_method")
{
    vessel_str_buf_clear();

    vessel_interpret("test", R"foo(
class Base {
  foo() {
    print "Base.foo()"
  }
}

class Derived is Base {
  foo() {
    print "Derived.foo()"
    super.foo()
  }
}

Derived().foo()
// expect: Derived.foo()
// expect: Base.foo()
)foo");
    REQUIRE(std::string(vessel_get_str_buf()) == R"(
Derived.foo()
Base.foo()
)" + 1);
}

TEST_CASE("closure")
{
    vessel_str_buf_clear();

    vessel_interpret("test", R"(
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
print closure() // expect: Base
)");
    REQUIRE(std::string(vessel_get_str_buf()) == R"(
Base
)" + 1);
}

TEST_CASE("constructor")
{
    vessel_str_buf_clear();

    vessel_interpret("test", R"foo(
class Base {
  init(a, b) {
    print "Base.init(" + a + ", " + b + ")"
}
}

class Derived is Base{
  init() {
    print "Derived.init()"
    super.init("a", "b")
  }
}

Derived()
// expect: Derived.init()
// expect: Base.init(a, b)
)foo");
    REQUIRE(std::string(vessel_get_str_buf()) == R"(
Derived.init()
Base.init(a, b)
)" + 1);
}

TEST_CASE("indirectly_inherited")
{
    vessel_str_buf_clear();

    vessel_interpret("test", R"foo(
class A {
  foo() {
    print "A.foo()"
  }
}

class B is A {}

class C is B {
  foo() {
    print "C.foo()"
    super.foo()
  }
}

C().foo()
// expect: C.foo()
// expect: A.foo()
)foo");
    REQUIRE(std::string(vessel_get_str_buf()) == R"(
C.foo()
A.foo()
)" + 1);
}

TEST_CASE("reassign_superclass")
{
    vessel_str_buf_clear();

    vessel_interpret("test", R"foo(
class Base {
  method() {
    print "Base.method()"
  }
}

class Derived is Base {
  method() {
    super.method()
  }
}

class OtherBase {
  method() {
    print "OtherBase.method()"
  }
}

var derived = Derived()
derived.method() // expect: Base.method()
Base = OtherBase
derived.method() // expect: Base.method()
)foo");
    REQUIRE(std::string(vessel_get_str_buf()) == R"(
Base.method()
Base.method()
)" + 1);
}

TEST_CASE("super_in_closure_in_inherited_method")
{
    vessel_str_buf_clear();

    vessel_interpret("test", R"foo(
class A {
  say() {
    print "A"
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
    print "B"
  }
}

class C is B {
  say() {
    print "C"
  }
}

C().getClosure()() // expect: A
)foo");
    REQUIRE(std::string(vessel_get_str_buf()) == R"(
A
)" + 1);
}

TEST_CASE("super_in_inherited_method")
{
    vessel_str_buf_clear();

    vessel_interpret("test", R"foo(
class A {
  say() {
    print "A"
  }
}

class B is A {
  test() {
    super.say()
  }

  say() {
    print "B"
  }
}

class C is B {
  say() {
    print "C"
  }
}

C().test() // expect: A
)foo");
    REQUIRE(std::string(vessel_get_str_buf()) == R"(
A
)" + 1);
}

TEST_CASE("this_in_superclass_method")
{
    vessel_str_buf_clear();

    vessel_interpret("test", R"foo(
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
print derived.a // expect: a
print derived.b // expect: b
)foo");
    REQUIRE(std::string(vessel_get_str_buf()) == R"(
a
b
)" + 1);
}