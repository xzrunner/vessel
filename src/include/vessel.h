#ifdef __cplusplus
extern "C"
{
#endif

#ifndef vessel_h
#define vessel_h

// Called after load_module_fn is called for module [name]. The original returned result
// is handed back to you in this callback, so that you can free memory if appropriate.
typedef void (*LoadModuleCompleteFn)(const char* name, struct LoadModuleResult result);

// The result of a load_module_fn call.
// [source] is the source code for the module, or NULL if the module is not found.
// [onComplete] an optional callback that will be called once Wren is done with the result.
typedef struct LoadModuleResult
{
	const char* source;
	LoadModuleCompleteFn on_complete;
	void* user_data;
} LoadModuleResult;

// Loads and returns the source code for the module [name].
typedef LoadModuleResult(*LoadModuleFn)(const char* name);

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
  LoadModuleFn load_module_fn;
} Configuration;

typedef void (*ForeignMethodFn)();
typedef void (*FinalizerFn)(void* data);

typedef struct
{
	// The callback invoked when the foreign object is created.
	//
	// This must be provided. Inside the body of this, it must call
	// [wrenSetSlotNewForeign()] exactly once.
	ForeignMethodFn allocate;

	// The callback invoked when the garbage collector is about to collect a
	// foreign object's memory.
	//
	// This may be `NULL` if the foreign class does not need to finalize.
	FinalizerFn finalize;
} ForeignClassMethods;

// Returns the number of slots available to the current foreign method.
int GetSlotCount();

// Reads a number from [slot].
//
// It is an error to call this if the slot does not contain a number.
double GetSlotDouble(int slot);

// Reads a foreign object from [slot] and returns a pointer to the foreign data
// stored with it.
//
// It is an error to call this if the slot does not contain an instance of a
// foreign class.
void* GetSlotForeign(int slot);

// Stores the numeric [value] in [slot].
void SetSlotDouble(int slot, double value);

// Creates a new instance of the foreign class stored in [classSlot] with [size]
// bytes of raw storage and places the resulting object in [slot].
//
// This does not invoke the foreign class's constructor on the new instance. If
// you need that to happen, call the constructor from Wren, which will then
// call the allocator foreign method. In there, call this to create the object
// and then the constructor will be invoked when the allocator returns.
//
// Returns a pointer to the foreign object's data.
void* SetSlotNewForeign(int slot, int classSlot, size_t size);

typedef enum
{
	INTERPRET_OK,
	INTERPRET_COMPILE_ERROR,
	INTERPRET_RUNTIME_ERROR
} InterpretResult;

InterpretResult interpret(const char* module, const char* source);

void ves_str_buf_clear();
const char* ves_get_str_buf();

void init_vm();
void free_vm();

#endif // vessel_h

#ifdef __cplusplus
}
#endif