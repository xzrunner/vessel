#ifdef __cplusplus
extern "C"
{
#endif

#ifndef vessel_vm_h
#define vessel_vm_h

#include "common.h"
#include "value.h"
#include "object.h"

#define FRAMES_MAX 64
#define STACK_MAX (FRAMES_MAX * UINT8_COUNT)

typedef struct
{
	ObjClosure* closure;
	uint8_t* ip;
	Value* slots;
} CallFrame;

typedef struct
{
	ObjClass* list_class;
	ObjClass* map_class;

	CallFrame frames[FRAMES_MAX];
	int frame_count;

	Value stack[STACK_MAX];
	Value* stack_top;

	Table globals;
	Table strings;

	ObjString* init_string;
	ObjUpvalue* open_upvalues;

	size_t bytes_allocated;
	size_t next_gc;

	Obj* objects;

	Table modules;

	int gray_count;
	int gray_capacity;
	Obj** gray_stack;

	ValueArray method_names;

	Value error;
} VM;

typedef enum
{
	INTERPRET_OK,
	INTERPRET_COMPILE_ERROR,
	INTERPRET_RUNTIME_ERROR
} InterpretResult;

extern VM vm;

void init_vm();
void free_vm();

InterpretResult interpret(const char* module, const char* source);

void push(Value value);
Value pop();

#endif // vessel_vm_h

#ifdef __cplusplus
}
#endif