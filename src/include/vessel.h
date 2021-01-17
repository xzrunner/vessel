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
// [onComplete] an optional callback that will be called once Wren is done with the result.
typedef struct VesselLoadModuleResult
{
	const char* source;
	VesselLoadModuleCompleteFn on_complete;
	void* user_data;
} VesselLoadModuleResult;

// Loads and returns the source code for the module [name].
typedef VesselLoadModuleResult(*VesselLoadModuleFn)(const char* name);

typedef struct
{
  // The callback Wren uses to load a module.
  //
  // Since Wren does not talk directly to the file system, it relies on the
  // embedder to physically locate and read the source code for a module. The
  // first time an import appears, Wren will call this and pass in the name of
  // the module being imported. The VM should return the soure code for that
  // module. Memory for the source should be allocated using [reallocateFn] and
  // Wren will take ownership over it.
  //
  // This will only be called once for any given module name. Wren caches the
  // result internally so subsequent imports of the same module will use the
  // previous source and not call this.
  //
  // If a module with the given name could not be found by the embedder, it
  // should return NULL and Wren will report that as a runtime error.
  VesselLoadModuleFn load_module_fn;
} VesselConfiguration;

typedef void (*VesselForeignMethodFn)();
typedef void (*VesselFinalizerFn)(void* data);

typedef struct
{
	VesselForeignMethodFn allocate;
	VesselFinalizerFn finalize;
} VesselForeignClassMethods;

// Returns the number of slots available to the current foreign method.
int vessel_get_slot_count();

// Reads a number from [slot].
//
// It is an error to call this if the slot does not contain a number.
double vessel_get_slot_double(int slot);

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
// you need that to happen, call the constructor from Wren, which will then
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

void vessel_init_vm();
void vessel_free_vm();

#endif // vessel_h

#ifdef __cplusplus
}
#endif