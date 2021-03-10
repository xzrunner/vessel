#ifdef __cplusplus
extern "C"
{
#endif

#ifndef vessel_h
#define vessel_h

#include <stdbool.h>
#include <stdint.h>

// Called after load_module_fn is called for module [name]. The original returned result
// is handed back to you in this callback, so that you can free memory if appropriate.
typedef void (*VesselLoadModuleCompleteFn)(const char* name, struct VesselLoadModuleResult result);

// The result of a load_module_fn call.
// [source] is the source code for the module, or NULL if the module is not found.
// [onComplete] an optional callback that will be called once Vessel is done with the result.
typedef struct VesselLoadModuleResult
{
	const char* source;
	VesselLoadModuleCompleteFn on_complete;
	void* user_data;
} VesselLoadModuleResult;

typedef void (*VesselForeignMethodFn)();
typedef int (*VesselFinalizerFn)(void* data);

// Loads and returns the source code for the module [name].
typedef VesselLoadModuleResult(*VesselLoadModuleFn)(const char* name);

// Returns a pointer to a foreign method on [className] in [module] with
// [signature].
typedef VesselForeignMethodFn(*VesselBindForeignMethodFn)(
	const char* module, const char* className, bool isStatic,
	const char* signature);

typedef struct
{
	VesselForeignMethodFn allocate;
	VesselFinalizerFn finalize;
} VesselForeignClassMethods;

// Returns a pair of pointers to the foreign methods used to allocate and
// finalize the data for instances of [className] in resolved [module].
typedef VesselForeignClassMethods(*VesselBindForeignClassFn)(
	const char* module, const char* className);

// Displays a string of text to the user.
typedef void (*VesselWriteFn)(const char* text);

typedef struct
{
  // The callback Vessel uses to load a module.
  //
  // Since Vessel does not talk directly to the file system, it relies on the
  // embedder to physically locate and read the source code for a module. The
  // first time an import appears, Vessel will call this and pass in the name of
  // the module being imported. The VM should return the soure code for that
  // module. Memory for the source should be allocated using [reallocateFn] and
  // Vessel will take ownership over it.
  //
  // This will only be called once for any given module name. Vessel caches the
  // result internally so subsequent imports of the same module will use the
  // previous source and not call this.
  //
  // If a module with the given name could not be found by the embedder, it
  // should return NULL and Vessel will report that as a runtime error.
  VesselLoadModuleFn load_module_fn;

  // The callback Vessel uses to find a foreign method and bind it to a class.
  //
  // When a foreign method is declared in a class, this will be called with the
  // foreign method's module, class, and signature when the class body is
  // executed. It should return a pointer to the foreign function that will be
  // bound to that method.
  //
  // If the foreign function could not be found, this should return NULL and
  // Vessel will report it as runtime error.
  VesselBindForeignMethodFn bind_foreign_method_fn;

  // The callback Vessel uses to find a foreign class and get its foreign methods.
  //
  // When a foreign class is declared, this will be called with the class's
  // module and name when the class body is executed. It should return the
  // foreign functions uses to allocate and (optionally) finalize the bytes
  // stored in the foreign object when an instance is created.
  VesselBindForeignClassFn bind_foreign_class_fn;

  VesselWriteFn write_fn;

} VesselConfiguration;

typedef enum
{
	VES_INTERPRET_OK,
	VES_INTERPRET_COMPILE_ERROR,
	VES_INTERPRET_RUNTIME_ERROR
} VesselInterpretResult;

VesselInterpretResult ves_interpret(const char* module, const char* source);

void* ves_compile(const char* module, const char* source);
VesselInterpretResult ves_run(void* closure);

void ves_set_config(VesselConfiguration* cfg);

void ves_init_vm();
void ves_free_vm();

int ves_gettop();

typedef enum
{
	VES_TYPE_BOOL,
	VES_TYPE_NUM,
	VES_TYPE_FOREIGN,
	VES_TYPE_LIST,
	VES_TYPE_MAP,
	VES_TYPE_SET,
	VES_TYPE_NULL,
	VES_TYPE_STRING,
	VES_TYPE_INSTANCE,

	// The object is of a type that isn't accessible by the C API.
	VES_TYPE_UNKNOWN
} VesselType;

VesselType ves_type(int index);

int ves_len(int index);

void ves_geti(int index, int i);
void ves_seti(int index, int i);

double ves_tonumber(int index);
bool ves_toboolean(int index);
const char* ves_tostring(int index);
void* ves_toforeign(int index);

double ves_optnumber(int index, double d);
bool ves_optboolean(int index, bool d);
const char* ves_optstring(int index, const char* d);

void ves_pushnumber(double n);
void ves_pushboolean(int b);
void ves_pushstring(const char* s);
void ves_pushlstring(const char* s, size_t len);

void ves_pop(int n);

int ves_getfield(int index, const char* k);
int ves_getglobal(const char* name);

VesselInterpretResult ves_call(int nargs, int nresults);

void ves_set_nil(int slot);
void ves_set_number(int slot, double value);
void ves_set_boolean(int slot, bool value);
void ves_set_lstring(int slot, const char* s, size_t len);
void* ves_set_newforeign(int slot, int class_slot, size_t size);

void ves_newlist(int num_elements);

#endif // vessel_h

#ifdef __cplusplus
}
#endif