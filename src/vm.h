#ifndef vessel_vm_h
#define vessel_vm_h

#include "common.h"
#include "value.h"
#include "object.h"
#include "vessel.h"

#define FRAMES_MAX 64
#define STACK_MAX (FRAMES_MAX * UINT8_COUNT)

typedef enum
{
#define OPCODE(name) OP_##name,
#include "opcodes.h"
#undef OPCODE
} OpCode;

typedef struct
{
	ObjClosure* closure;
	uint8_t* ip;
	Value* slots;
} CallFrame;

typedef struct
{
	ObjClass* bool_class;
	ObjClass* class_class;
	ObjClass* list_class;
	ObjClass* map_class;
	ObjClass* set_class;
	ObjClass* range_class;
	ObjClass* num_class;
	ObjClass* null_class;
	ObjClass* string_class;
	ObjClass* object_class;
	ObjClass* system_class;
	ObjClass* basic_class;

	ObjModule* last_module;

	CallFrame frames[FRAMES_MAX];
	int frame_count;
	int frame_count_begin;

	Value stack[STACK_MAX];
	Value* stack_top;

	Table strings;

	ObjString* init_str;
	ObjString* allocate_str;
	ObjString* finalize_str;
	ObjUpvalue* open_upvalues;

	size_t bytes_allocated;
	size_t next_gc;

	Obj* objects;

	Table modules;

	int gray_count;
	int gray_capacity;
	Obj** gray_stack;

	Value* api_stack;

	VesselConfiguration config;

	ValueArray method_names;

	Value error;
} VM;

extern VM vm;

// Adds a new top-level variable named [name] to [module], and optionally
// populates line with the line of the implicit first use (line can be NULL).
//
// Returns the symbol for the new variable, -1 if a variable with the given name
// is already defined, or -2 if there are too many variables defined.
// Returns -3 if this is a top-level lowercase variable (localname) that was
// used before being defined.
int DefineVariable(ObjModule* module, const char* name, size_t length, Value value, int* line);

int FinalizeForeign(ObjForeign* foreign);

void push(Value value);
Value pop();

#endif // vessel_vm_h
