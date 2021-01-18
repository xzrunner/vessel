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
typedef void (*VesselFinalizerFn)(void* data);

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
} VesselConfiguration;

int vessel_get_slot_count();

typedef enum
{
	VESSEL_TYPE_BOOL,
	VESSEL_TYPE_NUM,
	VESSEL_TYPE_FOREIGN,
	VESSEL_TYPE_LIST,
	VESSEL_TYPE_MAP,
	VESSEL_TYPE_NULL,
	VESSEL_TYPE_STRING,

	// The object is of a type that isn't accessible by the C API.
	VESSEL_TYPE_UNKNOWN
} VesselType;

VesselType vessel_get_slot_type(int slot);

bool vessel_get_slot_bool(int slot);
double vessel_get_slot_double(int slot);
const char* vessel_get_slot_string(int slot);

int vessel_get_list_count(int slot);
void vessel_get_list_element(int listSlot, int index, int elementSlot);

void vessel_get_map_value(int mapSlot, const char* key, int valueSlot);

// Reads a foreign object from [slot] and returns a pointer to the foreign data
// stored with it.
//
// It is an error to call this if the slot does not contain an instance of a
// foreign class.
void* vessel_get_slot_foreign(int slot);

// Stores the numeric [value] in [slot].
void vessel_set_slot_double(int slot, double value);

// Creates a new instance of the foreign class stored in [classSlot] with [size]
// bytes of raw storage and places the resulting object in [slot].
//
// This does not invoke the foreign class's constructor on the new instance. If
// you need that to happen, call the constructor from Vessel, which will then
// call the allocator foreign method. In there, call this to create the object
// and then the constructor will be invoked when the allocator returns.
//
// Returns a pointer to the foreign object's data.
void* vessel_set_slot_new_foreign(int slot, int classSlot, size_t size);

typedef enum
{
	VESSEL_INTERPRET_OK,
	VESSEL_INTERPRET_COMPILE_ERROR,
	VESSEL_INTERPRET_RUNTIME_ERROR
} VesselInterpretResult;

VesselInterpretResult vessel_interpret(const char* module, const char* source);

void vessel_str_buf_clear();
const char* vessel_get_str_buf();

void vessel_set_config(VesselConfiguration* cfg);

void vessel_init_vm();
void vessel_free_vm();

#endif // vessel_h

#ifdef __cplusplus
}
#endif