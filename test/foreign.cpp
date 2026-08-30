#include "utility.h"

#include <catch2/catch_test_macros.hpp>

#include <vessel.h>

#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

// Defined in test/utility.cpp (declared here only: utility.h stays a plain
// output-buffer helper so the other test files are untouched).
void config_foreign_vm(VesselBindForeignClassFn bind_class,
                       VesselBindForeignMethodFn bind_method);

namespace
{

struct BoxData
{
    double value;
    int receiver_tag;
};

// Finalizer accounting is per-instance: record the exact allocated data
// pointers and the exact finalized data pointers, then compare as pointer
// VALUES only (never dereference after free).
std::vector<std::uintptr_t> g_fin_allocated;
std::vector<std::uintptr_t> g_fin_finalized;

// The allocator sees the constructor arguments after the receiver: slot 0
// is the (still class-valued) receiver, slot 1+ are the ctor arguments.
void box_allocate()
{
    BoxData* box = (BoxData*)ves_set_newforeign(0, 0, sizeof(BoxData));
    box->value = ves_argnum() >= 1 ? ves_tonumber(1) : 0.0;
    box->receiver_tag = 0;
}

int box_finalize(void* data)
{
    (void)data;
    return (int)sizeof(BoxData);
}

// "Fin" is bound ONLY for the finalizer test's module, so its allocated/
// finalized pointer sets are isolated from every other test's objects.
void fin_allocate()
{
    void* data = ves_set_newforeign(0, 0, sizeof(BoxData));
    g_fin_allocated.push_back(reinterpret_cast<std::uintptr_t>(data));
}

int fin_finalize(void* data)
{
    g_fin_finalized.push_back(reinterpret_cast<std::uintptr_t>(data));
    return (int)sizeof(BoxData);
}

// Instance methods: the receiver is the foreign object in slot 0.
void box_value()
{
    BoxData* box = (BoxData*)ves_toforeign(0);
    box->receiver_tag = 1;
    ves_set_number(0, box->value);
}

void box_add()
{
    BoxData* box = (BoxData*)ves_toforeign(0);
    box->receiver_tag = 1;
    ves_set_number(0, box->value + ves_tonumber(1));
}

void box_tag()
{
    BoxData* box = (BoxData*)ves_toforeign(0);
    // Fails the whole test through the printed value if slot 0 is not the
    // receiver: reading the argument (or garbage) as BoxData misaligns the
    // tag, and NULL data crashes instead.
    ves_set_number(0, box->receiver_tag);
}

void box_receiver_is_foreign()
{
    // Keep this check non-dereferencing so the regression reports a stable
    // value when a super call exposes argument 1 instead of the receiver at
    // slot 0. The result also proves that invoke_from_class() propagated the
    // foreign callback's return slot back to the caller.
    ves_set_number(0, ves_type(0) == VES_TYPE_FOREIGN ? 1 : 0);
}

void box_static_seven()
{
    ves_set_number(0, 7);
}

VesselForeignClassMethods bind_class(const char* module, const char* class_name)
{
    VesselForeignClassMethods methods;
    methods.allocate = NULL;
    methods.finalize = NULL;
    if (std::strcmp(class_name, "Box") == 0)
    {
        // Box binds in any module so each test can use its own module
        // name (isolating module-level variables between tests).
        methods.allocate = box_allocate;
        methods.finalize = box_finalize;
    }
    else if (std::strcmp(module, "ffin") == 0 && std::strcmp(class_name, "Fin") == 0)
    {
        methods.allocate = fin_allocate;
        methods.finalize = fin_finalize;
    }
    // "Ghost" and anything else: no allocator, no finalizer.
    return methods;
}

VesselForeignMethodFn bind_method(const char* module, const char* class_name,
                                  bool is_static, const char* signature)
{
    (void)module;
    if (std::strcmp(class_name, "Box") == 0)
    {
        if (!is_static)
        {
            if (std::strcmp(signature, "value()") == 0) return box_value;
            if (std::strcmp(signature, "add(_)") == 0) return box_add;
            if (std::strcmp(signature, "tag()") == 0) return box_tag;
            if (std::strcmp(signature, "receiverIsForeign(_)") == 0)
            {
                return box_receiver_is_foreign;
            }
        }
        else if (std::strcmp(signature, "seven()") == 0)
        {
            return box_static_seven;
        }
    }
    // Ghost methods and unknown signatures are rejected.
    return NULL;
}

// RAII: foreign tests rewire the config; every test restores config_vm()
// so later test files keep the plain non-foreign configuration.
struct ForeignConfig
{
    ForeignConfig() { config_foreign_vm(bind_class, bind_method); }
    ~ForeignConfig() { config_vm(); }
};

// Recovery gate for every rejection scenario: after the failed run, the
// VM must still execute a known-good script, print exactly the expected
// bytes, and leave ves_gettop() at the height it had before the failing
// call - WITHOUT leaning on the next ves_run()'s "Stack not empty."
// reset to repair a dirty stack.
void verify_recovery(const char* module, const char* script,
                     const std::string& expected)
{
    const int top_before = ves_gettop();
    init_output_buf();
    REQUIRE(ves_interpret(module, script) == VES_INTERPRET_OK);
    REQUIRE(std::string(get_output_buf()) == expected);
    REQUIRE(ves_gettop() == top_before);
}

} // namespace

TEST_CASE("foreign_instance_method_receiver_and_arguments")
{
    init_output_buf();
    ForeignConfig cfg;

    VesselInterpretResult result = ves_interpret("frecv", R"(
foreign class Box {
  init(v) {}
  foreign value()
  foreign add(_)
  foreign tag()
  foreign static seven()
}

var b = Box(11)
System.print(b.value())  // expect: 11 (allocator read ctor arg in slot 1)
System.print(b.add(5))   // expect: 16 (receiver data + argument)
System.print(b.tag())    // expect: 1 (slot 0 was the receiver both calls)
System.print(Box.seven()) // expect: 7 (static foreign, no receiver data)
)");
    REQUIRE(result == VES_INTERPRET_OK);
    REQUIRE(std::string(get_output_buf()) == R"(
11
16
1
7
)" + 1);
}

TEST_CASE("foreign_super_call_preserves_receiver_and_return")
{
    init_output_buf();
    ForeignConfig cfg;

    VesselInterpretResult result = ves_interpret("fsuper", R"(
foreign class Box {
  init(v) {}
  foreign receiverIsForeign(_)
}

foreign class DerivedBox is Box {
  init(v) {}
  viaSuper(v) { return super.receiverIsForeign(v) }
}

var box = DerivedBox(11)
System.print(box.viaSuper(99))
)");
    REQUIRE(result == VES_INTERPRET_OK);
    REQUIRE(std::string(get_output_buf()) == "1\n");
}

TEST_CASE("foreign_return_value_keeps_stack_balanced")
{
    init_output_buf();
    ForeignConfig cfg;

    // ves_run() itself verifies the stack returns to its entry height
    // ("Stack not empty."), so OK here proves the foreign call collapsed
    // [receiver, args...] back to a single result slot.
    const int top_before = ves_gettop();
    void* closure = ves_compile("fbal", R"(
foreign class Box {
  init(v) {}
  foreign value()
  foreign add(_)
}

var b = Box(2)
var sink = b.add(b.add(b.value())) // nested foreign calls, results consumed
System.print(sink) // expect: 6 (2 + (2 + 2))
)");
    REQUIRE(closure != NULL);
    REQUIRE(ves_run(closure) == VES_INTERPRET_OK);
    REQUIRE(ves_gettop() == top_before);
    REQUIRE(std::string(get_output_buf()) == R"(
6
)" + 1);
}

TEST_CASE("foreign_class_without_allocator_is_runtime_error")
{
    init_output_buf();
    ForeignConfig cfg;

    const int top_before = ves_gettop();

    // Ghost binds no allocator: constructing it must report a runtime
    // error - in Release too, where the old ASSERT(find, ...) compiled
    // out and called through an uninitialized method value. Ghost
    // declares no foreign methods so the class body itself binds cleanly
    // and the failure happens at OP_FOREIGN_CONSTRUCT.
    VesselInterpretResult result = ves_interpret("fghost", R"(
foreign class Ghost {
  init() {}
}

var g = Ghost()
System.print("unreachable")
)");
    REQUIRE(result == VES_INTERPRET_RUNTIME_ERROR);
    REQUIRE(std::strstr(get_output_buf(), "unreachable") == NULL);
    // The interpreter reports its own error; it must NOT also have to
    // recover from a dirty stack (which would surface as the next
    // ves_run() printing "Stack not empty." to stderr).
    REQUIRE(ves_gettop() == top_before);

    verify_recovery("fghost-revive", R"(
System.print("alive")
System.print(2 + 3)
)", std::string(R"(
alive
5
)" + 1));
}

TEST_CASE("foreign_class_without_init_has_no_constructor")
{
    init_output_buf();
    ForeignConfig cfg;

    const int top_before = ves_gettop();

    // Box has a valid allocator but declares NO init, so the compiler
    // synthesizes no metaclass constructor: constructing must fail as a
    // runtime error instead of falling through to new_instance(), which
    // would build a plain ObjInstance under the foreign class and make
    // every later ves_toforeign() misread its bytes.
    VesselInterpretResult result = ves_interpret("fnoinit", R"(
foreign class Box {
  foreign value()
}

var b = Box()
System.print("unreachable")
)");
    REQUIRE(result == VES_INTERPRET_RUNTIME_ERROR);
    REQUIRE(std::strstr(get_output_buf(), "unreachable") == NULL);
    REQUIRE(ves_gettop() == top_before);

    verify_recovery("fnoinit-revive", R"(
System.print("alive")
)", std::string(R"(
alive
)" + 1));
}

TEST_CASE("foreign_static_and_unknown_method_rejected")
{
    init_output_buf();
    ForeignConfig cfg;

    const int top_before = ves_gettop();

    // The binder rejects every Ghost method, so executing the class body
    // (which binds foreign methods) must fail as a runtime error - and,
    // since runtime_error() resets the data stack, the binding failure
    // must not pop below the (now empty) stack afterwards.
    VesselInterpretResult result = ves_interpret("frej1", R"(
foreign class Ghost {
  foreign static forbidden()
}
System.print("unreachable")
)");
    REQUIRE(result == VES_INTERPRET_RUNTIME_ERROR);
    REQUIRE(std::strstr(get_output_buf(), "unreachable") == NULL);
    REQUIRE(ves_gettop() == top_before);

    verify_recovery("frej1-revive", R"(
System.print("alive")
)", std::string(R"(
alive
)" + 1));

    init_output_buf();
    // Unknown instance signature on a bound class is rejected as well.
    result = ves_interpret("frej2", R"(
foreign class Box {
  init(v) {}
  foreign missing()
}
System.print("unreachable")
)");
    REQUIRE(result == VES_INTERPRET_RUNTIME_ERROR);
    REQUIRE(std::strstr(get_output_buf(), "unreachable") == NULL);
    REQUIRE(ves_gettop() == top_before);

    verify_recovery("frej2-revive", R"(
System.print("alive")
)", std::string(R"(
alive
)" + 1));
}

TEST_CASE("foreign_finalizer_runs_once_per_instance_on_free_vm")
{
    init_output_buf();
    VesselInterpretResult result = VES_INTERPRET_OK;
    {
        ForeignConfig cfg;
        g_fin_allocated.clear();
        g_fin_finalized.clear();
        g_fin_allocated.reserve(2);
        g_fin_finalized.reserve(2);

        result = ves_interpret("ffin", R"(
foreign class Fin {
  init(v) {}
}

var a = Fin(1)
var b = Fin(2)
System.print("allocated")
)");
    }

    // ves_free_vm() tears down every object. Fin is bound only for this
    // module, so exactly two Fin allocations exist and each data pointer
    // must be finalized exactly once - no double finalize, no leak.
    config_foreign_vm(bind_class, bind_method);
    ves_free_vm();

    ves_init_vm();
    config_vm();
    init_output_buf();
    const bool revived = ves_interpret("test", "System.print(1)") == VES_INTERPRET_OK;

    REQUIRE(result == VES_INTERPRET_OK);
    REQUIRE(std::string(get_output_buf()) == "1\n");
    REQUIRE(revived);

    REQUIRE(g_fin_allocated.size() == 2);
    REQUIRE(g_fin_finalized.size() == 2);
    // The callbacks captured integer identities while each allocation was
    // still live, so no invalid pointer value is used after free_vm().
    for (std::uintptr_t finalized : g_fin_finalized)
    {
        int seen = 0;
        for (std::uintptr_t allocated : g_fin_allocated)
        {
            if (finalized == allocated)
            {
                ++seen;
            }
        }
        REQUIRE(seen == 1); // no unknown pointer was finalized
    }
    for (std::uintptr_t allocated : g_fin_allocated)
    {
        int seen = 0;
        for (std::uintptr_t finalized : g_fin_finalized)
        {
            if (finalized == allocated)
            {
                ++seen;
            }
        }
        REQUIRE(seen == 1); // each instance finalized exactly once
    }
}
