#include "compiler.h"
#include "memory.h"
#include "vm.h"
#include "debug.h"

#ifdef DEBUG_LOG_GC
#include <stdio.h>
#include "debug.h"
#endif

#include <stdlib.h>

#define GC_HEAP_GROW_FACTOR 2

void* reallocate(void* pointer, size_t old_size, size_t new_size)
{
	vm.bytes_allocated += new_size - old_size;

	if (new_size > old_size)
	{
#ifdef DEBUG_STRESS_GC
		collect_garbage();
#endif
		if (vm.bytes_allocated > vm.next_gc) {
			collect_garbage();
		}
	}

	if (new_size == 0) {
		free(pointer);
		return NULL;
	}

	void* result = realloc(pointer, new_size);
	if (result == NULL) {
		exit(1);
	}
	return result;
}

void mark_object(Obj* object)
{
	if (object == NULL) {
		return;
	}
	if (object->is_marked) {
		return;
	}

#ifdef DEBUG_LOG_GC
	printf("%p mark ", (void*)object);
	dump_value(OBJ_VAL(object), true);
	printf("\n");
#endif

	object->is_marked = true;

	if (vm.gray_capacity < vm.gray_count + 1)
	{
		vm.gray_capacity = GROW_CAPACITY(vm.gray_capacity);
		vm.gray_stack = realloc(vm.gray_stack, sizeof(Obj*) * vm.gray_capacity);

		if (vm.gray_stack == NULL) {
			exit(1);
		}
	}

	vm.gray_stack[vm.gray_count++] = object;
}

void mark_value(Value value)
{
	if (!IS_OBJ(value)) {
		return;
	}
	mark_object(AS_OBJ(value));
}

static void mark_array(ValueArray* array)
{
	for (int i = 0; i < array->count; i++) {
		mark_value(array->values[i]);
	}
}

static void blacken_object(Obj* object)
{
#ifdef DEBUG_LOG_GC
	printf("%p blacken ", (void*)object);
	dump_value(OBJ_VAL(object), true);
	printf("\n");
#endif

	switch (object->type)
	{
	case OBJ_BOUND_METHOD:
	{
		ObjBoundMethod* bound = (ObjBoundMethod*)object;
		mark_value(bound->receiver);
		mark_object((Obj*)bound->method);
		break;
	}
	case OBJ_CLASS:
	{
		ObjClass* klass = (ObjClass*)object;
		mark_object((Obj*)klass->obj.class_obj);
		mark_object((Obj*)klass->superclass);
		mark_object((Obj*)klass->name);
		mark_table(&klass->methods);
		break;
	}
	case OBJ_CLOSURE:
	{
		ObjClosure* closure = (ObjClosure*)object;
		mark_object((Obj*)closure->function);
		for (int i = 0; i < closure->upvalue_count; i++) {
			mark_object((Obj*)closure->upvalues[i]);
		}
		break;
	}
	case OBJ_METHOD:
	{
		ObjMethod* method = (ObjMethod*)object;
		if (method->type == METHOD_BLOCK) {
			mark_object((Obj*)method->as.closure);
		}
		break;
	}
	case OBJ_FUNCTION:
	{
		ObjFunction* function = (ObjFunction*)object;
		mark_object((Obj*)function->name);
		mark_array(&function->chunk.constants);
		break;
	}
	case OBJ_FOREIGN:
		break;
	case OBJ_INSTANCE:
	{
		ObjInstance* instance = (ObjInstance*)object;
		mark_object((Obj*)instance->klass);
		mark_table(&instance->fields);
		break;
	}
	case OBJ_NATIVE:
		break;
	case OBJ_STRING:
		break;
	case OBJ_UPVALUE:
		mark_value(((ObjUpvalue*)object)->closed);
		break;
	case OBJ_MODULE:
	{
		ObjModule* module = (ObjModule*)object;
		mark_array(&module->variables);
		mark_array(&module->variable_names);
		mark_object((Obj*)module->name);
	}
	break;
	case OBJ_LIST:
	{
		ObjList* list = (ObjList*)object;
		mark_array(&list->elements);
	}
		break;
	case OBJ_MAP:
	{
		ObjMap* map = (ObjMap*)object;
		mark_table(&map->entries);
	}
		break;
	case OBJ_SET:
	{
		ObjSet* set = (ObjSet*)object;
		mark_array(&set->elements);
	}
		break;
	case OBJ_RANGE:
		break;
	default:
		ASSERT(0, "unknown obj type.");
	}
}

static void free_object(Obj* object)
{
#ifdef DEBUG_LOG_GC
	printf("%p free type %d\n", (void*)object, object->type);
#endif

	switch (object->type)
	{
	case OBJ_BOUND_METHOD:
		FREE(ObjBoundMethod, object);
		break;
	case OBJ_CLASS:
	{
		ObjClass* klass = (ObjClass*)object;
		free_table(&klass->methods);
		FREE(ObjClass, object);
		break;
	}
	case OBJ_CLOSURE:
	{
		ObjClosure* closure = (ObjClosure*)object;
		FREE_ARRAY(ObjUpvalue*, closure->upvalues, closure->upvalue_count);
		FREE(ObjClosure, object);
		break;
	}
	case OBJ_METHOD:
		FREE(ObjMethod, object);
		break;
	case OBJ_FUNCTION:
	{
		ObjFunction* function = (ObjFunction*)object;
		free_chunk(&function->chunk);
		FREE(ObjFunction, object);
		break;
	}
	case OBJ_FOREIGN:
	{
		int size = FinalizeForeign((ObjForeign*)object);
		reallocate(object, sizeof(ObjForeign) + size, 0);
	}
		break;
	case OBJ_INSTANCE:
	{
		ObjInstance* instance = (ObjInstance*)object;
		free_table(&instance->fields);
		FREE(ObjInstance, object);
		break;
	}
	case OBJ_NATIVE:
		FREE(ObjNative, object);
		break;
	case OBJ_STRING:
	{
		ObjString* string = (ObjString*)object;
		FREE_ARRAY(char, string->chars, string->length + 1);
		FREE(ObjString, object);
		break;
	}
	case OBJ_UPVALUE:
		FREE(ObjUpvalue, object);
		break;
	case OBJ_MODULE:
	{
		ObjModule* module = (ObjModule*)object;
		free_value_array(&module->variables);
		free_value_array(&module->variable_names);
		FREE(ObjModule, object);
		break;
	}
	case OBJ_LIST:
	{
		ObjList* list = (ObjList*)object;
		free_value_array(&list->elements);
		FREE(ObjList, list);
		break;
	}
	case OBJ_MAP:
	{
		ObjMap* map = (ObjMap*)object;
		free_table(&map->entries);
		FREE(ObjMap, map);
		break;
	}
	case OBJ_SET:
	{
		ObjSet* set = (ObjSet*)object;
		free_value_array(&set->elements);
		FREE(ObjSet, set);
		break;
	}
	case OBJ_RANGE:
		FREE(ObjRange, object);
		break;
	default:
		ASSERT(0, "unknown obj type.");
	}
}

static void mark_roots()
{
	mark_table(&vm.modules);

	// Temporary roots.
	for (int i = 0; i < vm.num_temp_roots; i++) {
		mark_object(vm.temp_roots[i]);
	}

	for (Value* slot = vm.stack; slot < vm.stack_top; slot++) {
		mark_value(*slot);
	}

	for (int i = 0; i < vm.frame_count; i++) {
		mark_object((Obj*)vm.frames[i].closure);
	}

	for (ObjUpvalue* upvalue = vm.open_upvalues;
		upvalue != NULL;
		upvalue = upvalue->next) {
		mark_object((Obj*)upvalue);
	}

	mark_table(&vm.modules);
	mark_compiler_roots();
	mark_object((Obj*)vm.init_str);
	mark_object((Obj*)vm.allocate_str);
	mark_object((Obj*)vm.finalize_str);
	mark_array(&vm.method_names);
}

static void trace_references()
{
	while (vm.gray_count > 0) {
		Obj* object = vm.gray_stack[--vm.gray_count];
		blacken_object(object);
	}
}

static void sweep()
{
	Obj* previous = NULL;
	Obj* object = vm.objects;
	while (object != NULL)
	{
		if (object->is_marked)
		{
			//> unmark
			object->is_marked = false;
			//< unmark
			previous = object;
			object = object->next;
		}
		else
		{
			Obj* unreached = object;

			object = object->next;
			if (previous != NULL) {
				previous->next = object;
			}
			else {
				vm.objects = object;
			}

			free_object(unreached);
		}
	}
}

void collect_garbage()
{
#ifdef DEBUG_LOG_GC
	printf("-- gc begin\n");
	size_t before = vm.bytes_allocated;
#endif

	mark_gray_compiler();
	mark_roots();
	trace_references();
	table_remove_white(&vm.strings);
	sweep();

	vm.next_gc = vm.bytes_allocated * GC_HEAP_GROW_FACTOR;

#ifdef DEBUG_LOG_GC
	printf("-- gc end\n");
	printf("   collected %ld bytes (from %ld to %ld) next at %ld\n",
		before - vm.bytes_allocated, before, vm.bytes_allocated,
		vm.next_gc);
#endif
}

void free_objects()
{
	Obj* object = vm.objects;
	while (object != NULL) {
		Obj* next = object->next;
		free_object(object);
		object = next;
	}

	free(vm.gray_stack);
}