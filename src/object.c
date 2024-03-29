#include "object.h"
#include "vm.h"
#include "memory.h"
#include "utils.h"
#ifdef STATISTICS
#include "statistics.h"
#endif // STATISTICS

#include <stdio.h>

#define ALLOCATE_OBJ(type, objectType) \
    (type*)allocate_object(sizeof(type), objectType)

#define ALLOCATE_FLEX(mainType, objectType, arrayType, count)                          \
    ((mainType*)allocate_object(sizeof(mainType) + sizeof(arrayType) * (count), objectType))

static Obj* allocate_object(size_t size, ObjType type)
{
	Obj* object = (Obj*)reallocate(NULL, 0, size);
	object->type = type;
	object->is_marked = false;
	object->class_obj = NULL;
	object->next = vm.objects;
	vm.objects = object;

#ifdef DEBUG_LOG_GC
	printf("%p allocate %ld for %d\n", (void*)object, size, type);
#endif

	return object;
}

ObjBoundMethod* new_bound_method(Value receiver, ObjClosure* method)
{
	ObjBoundMethod* bound = ALLOCATE_OBJ(ObjBoundMethod, OBJ_BOUND_METHOD);
	bound->receiver = receiver;
	bound->method = method;
	return bound;
}

ObjClass* new_class(ObjClass* superclass, int num_fields, ObjString* name, ObjModule* module)
{
	// Create the metaclass.
//	Value metaclass_name = string_format("@ metaclass", OBJ_VAL(name));
	Value metaclass_name = string_format("@", OBJ_VAL(name));
	push(metaclass_name);

	ObjClass* metaclass = new_single_class(0, AS_STRING(metaclass_name), module);
	metaclass->obj.class_obj = vm.class_class;

	pop();

	// Make sure the metaclass isn't collected when we allocate the class.
	push(OBJ_VAL(metaclass));

	// Metaclasses always inherit Class and do not parallel the non-metaclass
	// hierarchy.
	bind_superclass(metaclass, vm.class_class);

	ObjClass* class_obj = new_single_class(num_fields, name, module);

	// Make sure the class isn't collected while the inherited methods are being
	// bound.
	push(OBJ_VAL(class_obj));

	class_obj->obj.class_obj = metaclass;
	bind_superclass(class_obj, superclass);

	pop();
	pop();

	return class_obj;
}

ObjClass* new_single_class(int num_fields, ObjString* name, ObjModule* module)
{
	ObjClass* klass = ALLOCATE_OBJ(ObjClass, OBJ_CLASS);
	klass->superclass = NULL;
	klass->module = module;
	klass->name = name;
	klass->num_fields = num_fields;
	init_table(&klass->methods);
	return klass;
}

ObjClosure* new_closure(ObjFunction* function)
{
	ObjUpvalue** upvalues = ALLOCATE(ObjUpvalue*, function->upvalue_count);
	for (int i = 0; i < function->upvalue_count; i++) {
		upvalues[i] = NULL;
	}

	ObjClosure* closure = ALLOCATE_OBJ(ObjClosure, OBJ_CLOSURE);
	closure->function = function;
	closure->upvalues = upvalues;
	closure->upvalue_count = function->upvalue_count;
	return closure;
}

ObjMethod* new_method()
{
	ObjMethod* method = ALLOCATE_OBJ(ObjMethod, OBJ_METHOD);

	method->type = METHOD_NONE;

#ifdef STATISTICS
	method->call_times = 0;
	method->run_time = 0;
	stat_add_callee(OBJ_VAL(method));
#endif // STATISTICS

	return method;
}

ObjFunction* new_function(ObjModule* module)
{
	ObjFunction* function = ALLOCATE_OBJ(ObjFunction, OBJ_FUNCTION);

	function->arity = 0;
	function->upvalue_count = 0;
	function->name = NULL;
	init_chunk(&function->chunk);
	function->module = module;

#ifdef STATISTICS
	function->call_times = 0;
	function->run_time = 0;
	stat_add_callee(OBJ_VAL(function));
#endif // STATISTICS
	
	return function;
}

ObjForeign* new_foreign(size_t size, ObjClass* klass)
{
	ObjForeign* foreign = ALLOCATE_FLEX(ObjForeign, OBJ_FOREIGN, uint8_t, size);
	foreign->obj.class_obj = klass;
	memset(foreign->data, 0, size);
	return foreign;
}

ObjInstance* new_instance(ObjClass* klass)
{
	ObjInstance* instance = ALLOCATE_OBJ(ObjInstance, OBJ_INSTANCE);
	instance->klass = klass;
	init_table(&instance->fields);
	return instance;
}

ObjNative* new_native(NativeFn function)
{
	ObjNative* native = ALLOCATE_OBJ(ObjNative, OBJ_NATIVE);
	native->function = function;
	return native;
}

ObjModule* new_module(ObjString* name)
{
	ObjModule* module = ALLOCATE_OBJ(ObjModule, OBJ_MODULE);
	init_value_array(&module->variables);
	init_value_array(&module->variable_names);
	module->name = name;
	return module;
}

ObjList* new_list(uint32_t num_elements)
{
	Value* elements = NULL;
	if (num_elements > 0) {
		elements = ALLOCATE(Value, num_elements);
	}

	ObjList* list = ALLOCATE_OBJ(ObjList, OBJ_LIST);
	list->obj.class_obj = vm.list_class;
	list->elements.capacity = num_elements;
	list->elements.count = num_elements;
	list->elements.values = elements;
	return list;
}

ObjMap* new_map()
{
	ObjMap* map = ALLOCATE_OBJ(ObjMap, OBJ_MAP);
	map->obj.class_obj = vm.map_class;
	init_table(&map->entries);
	return map;
}

ObjSet* new_set()
{
	ObjSet* set = ALLOCATE_OBJ(ObjSet, OBJ_SET);
	set->obj.class_obj = vm.set_class;
	init_value_array(&set->elements);
	return set;
}

ObjString* allocate_string(char* chars, int length, uint32_t hash)
{
	ObjString* string = ALLOCATE_OBJ(ObjString, OBJ_STRING);
	string->obj.class_obj = vm.string_class;
	string->length = length;
	string->chars = chars;
	string->hash = hash;

	push(OBJ_VAL(string));
	table_set(&vm.strings, string, NIL_VAL);
	pop();

	return string;
}

uint32_t hash_string(const char* key, int length)
{
	uint32_t hash = 2166136261u;

	for (int i = 0; i < length; i++) {
		hash ^= key[i];
		hash *= 16777619;
	}

	return hash;
}

ObjString* take_string(char* chars, int length)
{
	uint32_t hash = hash_string(chars, length);
	ObjString* interned = table_find_string(&vm.strings, chars, length, hash);
	if (interned != NULL) {
		FREE_ARRAY(char, chars, length + 1);
		return interned;
	}

	return allocate_string(chars, length, hash);
}

ObjString* copy_string(const char* chars, int length)
{
	uint32_t hash = hash_string(chars, length);
	ObjString* interned = table_find_string(&vm.strings, chars, length, hash);
	if (interned != NULL) {
		return interned;
	}

	char* heap_chars = ALLOCATE(char, length + 1);
	memcpy(heap_chars, chars, length);
	heap_chars[length] = '\0';

	return allocate_string(heap_chars, length, hash);
}

ObjUpvalue* new_upvalue(Value* slot)
{
	ObjUpvalue* upvalue = ALLOCATE_OBJ(ObjUpvalue, OBJ_UPVALUE);
	upvalue->closed = NIL_VAL;
	upvalue->location = slot;
	upvalue->next = NULL;
	return upvalue;
}

ObjRange* new_range()
{
	ObjRange* range = ALLOCATE_OBJ(ObjRange, OBJ_RANGE);
	range->obj.class_obj = vm.range_class;
	range->from = 0;
	range->to = 0;
	return range;
}

void bind_superclass(ObjClass* subclass, ObjClass* superclass)
{
	ASSERT(superclass != NULL, "Must have superclass.");

	subclass->superclass = superclass;

	if (subclass->num_fields != -1) {
		subclass->num_fields += superclass->num_fields;
	} else {
		ASSERT(superclass->num_fields <= 0, "A foreign class cannot inherit from a class with fields.");
	}

	table_add_all(&superclass->methods, &subclass->methods);
}

ObjClass* get_class(Value value)
{
	if (IS_INSTANCE(value)) {
		return AS_INSTANCE(value)->klass;
	} else if (IS_OBJ(value)) {
		return AS_OBJ(value)->class_obj;
	} else if (IS_BOOL(value)) {
		return vm.bool_class;
	} else if (IS_NUMBER(value)) {
		return vm.num_class;
	} else if (IS_NIL(value)) {
		return vm.null_class;
	} else if (IS_STRING(value)) {
		return vm.string_class;
	}
	return NULL;
}