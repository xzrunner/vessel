#ifndef vessel_object_h
#define vessel_object_h

#include "common.h"
#include "chunk.h"
#include "table.h"

#include <stdbool.h>

#define OBJ_TYPE(value)        (AS_OBJ(value)->type)

#define IS_METHOD(value)       is_obj_type(value, OBJ_METHOD)
#define IS_BOUND_METHOD(value) is_obj_type(value, OBJ_BOUND_METHOD)
#define IS_CLASS(value)        is_obj_type(value, OBJ_CLASS)
#define IS_CLOSURE(value)      is_obj_type(value, OBJ_CLOSURE)
#define IS_METHOD(value)       is_obj_type(value, OBJ_METHOD)
#define IS_FUNCTION(value)     is_obj_type(value, OBJ_FUNCTION)
#define IS_FOREIGN(value)      is_obj_type(value, OBJ_FOREIGN)
#define IS_INSTANCE(value)     is_obj_type(value, OBJ_INSTANCE)
#define IS_NATIVE(value)       is_obj_type(value, OBJ_NATIVE)
#define IS_STRING(value)       is_obj_type(value, OBJ_STRING)
#define IS_MODULE(value)       is_obj_type(value, OBJ_MODULE)
#define IS_LIST(value)         is_obj_type(value, OBJ_LIST)
#define IS_MAP(value)          is_obj_type(value, OBJ_MAP)

#define AS_METHOD(value)       ((ObjMethod*)AS_OBJ(value))
#define AS_BOUND_METHOD(value) ((ObjBoundMethod*)AS_OBJ(value))
#define AS_CLASS(value)        ((ObjClass*)AS_OBJ(value))
#define AS_CLOSURE(value)      ((ObjClosure*)AS_OBJ(value))
#define AS_FUNCTION(value)     ((ObjFunction*)AS_OBJ(value))
#define AS_FOREIGN(value)      ((ObjForeign*)AS_OBJ(value))
#define AS_INSTANCE(value)     ((ObjInstance*)AS_OBJ(value))
#define AS_NATIVE(value)       (((ObjNative*)AS_OBJ(value))->function)
#define AS_STRING(value)       ((ObjString*)AS_OBJ(value))
#define AS_CSTRING(value)      (((ObjString*)AS_OBJ(value))->chars)
#define AS_MODULE(value)       ((ObjModule*)AS_OBJ(value))
#define AS_LIST(value)         ((ObjList*)AS_OBJ(value))
#define AS_MAP(value)          ((ObjMap*)AS_OBJ(value))

typedef enum
{
	OBJ_BOUND_METHOD,
	OBJ_CLASS,
	OBJ_CLOSURE,
	OBJ_METHOD,
	OBJ_FUNCTION,
	OBJ_FOREIGN,
	OBJ_INSTANCE,
	OBJ_NATIVE,
	OBJ_STRING,
	OBJ_UPVALUE,
	OBJ_MODULE,
	OBJ_LIST,
	OBJ_MAP,
} ObjType;

typedef struct ObjClass ObjClass;

struct Obj
{
	ObjType type;
	bool is_marked;

	// The object's class.
	ObjClass* class_obj;

	struct Obj* next;
};

typedef struct ObjModule ObjModule;

typedef struct
{
	Obj obj;
	int arity;
	int upvalue_count;
	Chunk chunk;
	ObjString* name;
	ObjModule* module;
} ObjFunction;

typedef Value(*NativeFn)(int arg_count, Value* args);

typedef struct
{
	Obj obj;
	NativeFn function;
} ObjNative;

struct ObjString
{
	Obj obj;
	int length;
	char* chars;
	uint32_t hash;
};

typedef struct ObjUpvalue
{
	Obj obj;
	Value* location;
	Value closed;
	struct ObjUpvalue* next;
} ObjUpvalue;

typedef struct
{
	Obj obj;
	ObjFunction* function;
	ObjUpvalue** upvalues;
	int upvalue_count;
} ObjClosure;

typedef enum
{
	METHOD_PRIMITIVE,
	METHOD_FUNCTION_CALL,
	METHOD_FOREIGN,
	METHOD_BLOCK,
	METHOD_NONE
} MethodType;

typedef bool (*Primitive)(Value* args);
typedef void (*VesselForeignMethodFn)();

typedef struct
{
	Obj obj;
	MethodType type;
	union
	{
		Primitive primitive;
		VesselForeignMethodFn foreign;
		ObjClosure* closure;
	} as;
} ObjMethod;

struct ObjClass
{
	Obj obj;
	ObjClass* superclass;
	ObjString* name;
	int num_fields;
	Table methods;
};

typedef struct
{
	Obj obj;
	uint8_t data[FLEXIBLE_ARRAY];
} ObjForeign;

typedef struct
{
	Obj obj;
	ObjClass* klass;
	Table fields;
} ObjInstance;

typedef struct
{
	Obj obj;
	Value receiver;
	ObjClosure* method;
} ObjBoundMethod;

struct ObjModule
{
	Obj obj;
	ValueArray variables;
	ValueArray variable_names;
	ObjString* name;
};

typedef struct
{
	Obj obj;
	ValueArray elements;
} ObjList;

typedef struct
{
	Obj obj;
	Table entries;
} ObjMap;

ObjBoundMethod* new_bound_method(Value receiver, ObjClosure* method);
ObjClass* new_class(ObjClass* superclass, int num_fields, ObjString* name);
ObjClass* new_single_class(int num_fields, ObjString* name);
ObjClosure* new_closure(ObjFunction* function);
ObjMethod* new_method();
ObjFunction* new_function(ObjModule* module);
ObjForeign* new_foreign(size_t size, ObjClass* klass);
ObjInstance* new_instance(ObjClass* klass);
ObjNative* new_native(NativeFn function);
ObjModule* new_module(ObjString* name);
ObjList* new_list(uint32_t num_elements);
ObjMap* new_map();
uint32_t hash_string(const char* key, int length);
ObjString* allocate_string(char* chars, int length, uint32_t hash);
ObjString* take_string(char* chars, int length);
ObjString* copy_string(const char* chars, int length);
ObjUpvalue* new_upvalue(Value* slot);

void bind_superclass(ObjClass* subclass, ObjClass* superclass);

ObjClass* get_class(Value value);

static inline bool is_obj_type(Value value, ObjType type) {
	return IS_OBJ(value) && AS_OBJ(value)->type == type;
}

#endif // vessel_object_h
