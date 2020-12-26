#include "core.h"
#include "object.h"
#include "vm.h"
#include "primitive.h"
#include "utils.h"

DEF_PRIMITIVE(list_new)
{
	RETURN_OBJ(new_list(0));
}

DEF_PRIMITIVE(list_add)
{
	write_value_array(&AS_LIST(args[0])->elements, args[1]);
	RETURN_VAL(args[1]);
}

DEF_PRIMITIVE(list_addCore)
{
	write_value_array(&AS_LIST(args[0])->elements, args[1]);

	// Return the list.
	RETURN_VAL(args[0]);
}

DEF_PRIMITIVE(list_clear)
{
	free_value_array(&AS_LIST(args[0])->elements);
	RETURN_NULL;
}

DEF_PRIMITIVE(list_count)
{
	RETURN_NUM(AS_LIST(args[0])->elements.count);
}

DEF_PRIMITIVE(list_removeAt)
{
	ObjList* list = AS_LIST(args[0]);
	uint32_t index = validate_index(args[1], list->elements.count, "Index");
	if (index == UINT32_MAX) {
		return false;
	}

	RETURN_VAL(array_remove_at(&list->elements, index));
}

DEF_PRIMITIVE(list_subscript)
{
	ObjList* list = AS_LIST(args[0]);

	if (IS_NUMBER(args[1]))
	{
		uint32_t index = validate_index(args[1], list->elements.count, "Subscript");
		if (index == UINT32_MAX) {
			return false;
		}

		RETURN_VAL(list->elements.values[index]);
	}

	const int step = 1;
	const int count = list->elements.count;
	const int start = 0;

	//int step;
	//uint32_t count = list->elements.count;
	//uint32_t start = calculateRange(AS_RANGE(args[1]), &count, &step);
	//if (start == UINT32_MAX) {
	//	return false;
	//}

	ObjList* result = new_list(count);
	for (int i = 0; i < count; i++) {
		result->elements.values[i] = list->elements.values[start + i * step];
	}

	RETURN_OBJ(result);
}

DEF_PRIMITIVE(list_subscriptSetter)
{
	ObjList* list = AS_LIST(args[0]);
	uint32_t index = validate_index(args[1], list->elements.count, "Subscript");
	if (index == UINT32_MAX) {
		return false;
	}

	list->elements.values[index] = args[2];
	RETURN_VAL(args[2]);
}

DEF_PRIMITIVE(map_new)
{
  RETURN_OBJ(new_map());
}

DEF_PRIMITIVE(map_subscript)
{
	//if (!validate_key(args[1])) {
	//	return false;
	//}
	if (!IS_STRING(args[1])) {
		return false;
	}

	Value value;
	table_get(&AS_MAP(args[0])->entries, AS_STRING(args[1]), &value);

	RETURN_VAL(value);
}

DEF_PRIMITIVE(map_subscriptSetter)
{
	//if (!validate_key(args[1])) {
	//	return false;
	//}
	if (!IS_STRING(args[1])) {
		return false;
	}

	table_set(&AS_MAP(args[0])->entries, AS_STRING(args[1]), args[2]);
	RETURN_VAL(args[2]);
}

// Adds an entry to the map and then returns the map itself. This is called by
// the compiler when compiling map literals instead of using [_]=(_) to
// minimize stack churn.
DEF_PRIMITIVE(map_addCore)
{
	//if (!validate_key(args[1])) {
	//	return false;
	//}
	if (!IS_STRING(args[1])) {
		return false;
	}

	table_set(&AS_MAP(args[0])->entries, AS_STRING(args[1]), args[2]);

	// Return the map itself.
	RETURN_VAL(args[0]);
}

DEF_PRIMITIVE(map_clear)
{
	free_table(&AS_MAP(args[0])->entries);
	RETURN_NULL;
}

DEF_PRIMITIVE(map_containsKey)
{
	//if (!validate_key(args[1])) {
	//	return false;
	//}
	if (!IS_STRING(args[1])) {
		return false;
	}

	Value value;
	RETURN_BOOL(table_get(&AS_MAP(args[0])->entries, AS_STRING(args[1]), &value));
}

DEF_PRIMITIVE(map_count)
{
	RETURN_NUM(AS_MAP(args[0])->entries.count);
}

DEF_PRIMITIVE(map_iterate)
{
	ObjMap* map = AS_MAP(args[0]);

	if (map->entries.count == 0) {
		RETURN_FALSE;
	}

	// If we're starting the iteration, start at the first used entry.
	int index = 0;

	// Otherwise, start one past the last entry we stopped at.
	if (!IS_NIL(args[1]))
	{
		if (!validate_int(args[1], "Iterator")) {
			return false;
		}

		if (AS_NUMBER(args[1]) < 0) {
			RETURN_FALSE;
		}
		index = (uint32_t)AS_NUMBER(args[1]);

		if (index >= map->entries.capacity) {
			RETURN_FALSE;
		}

		// Advance the iterator.
		index++;
	}

	// Find a used entry, if any.
	for (; index < map->entries.capacity; index++)
	{
		if (map->entries.entries[index].key) {
			RETURN_NUM(index);
		}
	}

	// If we get here, walked all of the entries.
	RETURN_FALSE;
}

DEF_PRIMITIVE(map_remove)
{
	//if (!validate_key(args[1])) {
	//	return false;
	//}
	if (!IS_STRING(args[1])) {
		return false;
	}

	table_delete(&AS_MAP(args[0])->entries, AS_STRING(args[1]));
	//RETURN_VAL(table_delete(&AS_MAP(args[0])->entries, AS_STRING(args[1])));
	RETURN_NULL;
}

//DEF_PRIMITIVE(map_keyIteratorValue)
//{
//  ObjMap* map = AS_MAP(args[0]);
//  uint32_t index = validate_index(args[1], map->entries.capacity, "Iterator");
//  if (index == UINT32_MAX) {
//	  return false;
//  }
//
//  MapEntry* entry = &map->entries[index];
//  if (IS_UNDEFINED(entry->key))
//  {
//    RETURN_ERROR("Invalid map iterator.");
//  }
//
//  RETURN_VAL(entry->key);
//}
//
//DEF_PRIMITIVE(map_valueIteratorValue)
//{
//  ObjMap* map = AS_MAP(args[0]);
//  uint32_t index = validate_index(args[1], map->entries.capacity, "Iterator");
//  if (index == UINT32_MAX) return false;
//
//  MapEntry* entry = &map->entries[index];
//  if (IS_UNDEFINED(entry->key))
//  {
//    RETURN_ERROR("Invalid map iterator.");
//  }
//
//  RETURN_VAL(entry->value);
//}

static int define_variable(ObjModule* module, const char* name, size_t length, Value value)
{
	if (module->variables.count == MAX_MODULE_VARS) {
		return -2;
	}

	if (IS_OBJ(value)) {
		push(value);
	}

	int symbol = symbol_table_find(&module->variable_names, name, length);
	if (symbol == -1)
	{
		symbol = symbol_table_add(&module->variable_names, name, length);
		write_value_array(&module->variables, value);
	}
	else
	{
		symbol = -1;
	}

	if (IS_OBJ(value)) {
		pop();
	}

	return symbol;
}

static ObjClass* define_class(ObjModule* module, const char* name)
{
	ObjString* name_string = copy_string(name, strlen(name));
	push(OBJ_VAL(name_string));

	ObjClass* class_obj = new_single_class(0, name_string);

	define_variable(module, name, name_string->length, OBJ_VAL(class_obj));

	pop();
	return class_obj;
}

void initialize_core()
{
	ObjModule* core_module = new_module(NULL);

	push(OBJ_VAL(core_module));
	table_set(&vm.modules, copy_string("Core", 4), OBJ_VAL(core_module));
	pop();

	vm.list_class = new_class(copy_string("List", 4));
	define_variable(core_module, "List", 4, OBJ_VAL(vm.list_class));
	PRIMITIVE(vm.list_class->obj.class_obj, "new()", list_new);
	PRIMITIVE(vm.list_class, "[_]", list_subscript);
	PRIMITIVE(vm.list_class, "[_]=(_)", list_subscriptSetter);
	PRIMITIVE(vm.list_class, "add(_)", list_add);
	PRIMITIVE(vm.list_class, "addCore_(_)", list_addCore);
	PRIMITIVE(vm.list_class, "clear()", list_clear);
	PRIMITIVE(vm.list_class, "count", list_count);
	PRIMITIVE(vm.list_class, "removeAt(_)", list_removeAt);

	vm.map_class = new_class(copy_string("Map", 4));
	define_variable(core_module, "Map", 3, OBJ_VAL(vm.map_class));
	PRIMITIVE(vm.map_class->obj.class_obj, "new()", map_new);
	PRIMITIVE(vm.map_class, "[_]", map_subscript);
	PRIMITIVE(vm.map_class, "[_]=(_)", map_subscriptSetter);
	PRIMITIVE(vm.map_class, "addCore_(_,_)", map_addCore);
	PRIMITIVE(vm.map_class, "clear()", map_clear);
	PRIMITIVE(vm.map_class, "containsKey(_)", map_containsKey);
	PRIMITIVE(vm.map_class, "count", map_count);
	PRIMITIVE(vm.map_class, "remove(_)", map_remove);
	PRIMITIVE(vm.map_class, "iterate(_)", map_iterate);
	//PRIMITIVE(vm.map_class, "keyIteratorValue_(_)", map_keyIteratorValue);
	//PRIMITIVE(vm.map_class, "valueIteratorValue_(_)", map_valueIteratorValue);
}