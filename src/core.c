#include "core.h"
#include "object.h"
#include "vm.h"
#include "primitive.h"
#include "utils.h"
#include "core.ves.inc"

#include <math.h>

DEF_PRIMITIVE(object_is)
{
	if (!IS_CLASS(args[1]))
	{
		vm.error = OBJ_VAL(copy_string("Right operand must be a class.", sizeof("Right operand must be a class.") - 1));
		RETURN_BOOL(false);
	}

	ObjClass* class_obj = get_class(args[0]);
	ObjClass* base_class_obj = AS_CLASS(args[1]);

	// Walk the superclass chain looking for the class.
	do
	{
		if (base_class_obj == class_obj) {
			RETURN_BOOL(true);
		}

		class_obj = class_obj->superclass;
	} while (class_obj != NULL);

	RETURN_BOOL(false);
}

DEF_PRIMITIVE(object_type)
{
	RETURN_OBJ(get_class(args[0]));
}

static Value num_to_string(double value)
{
  // Edge case: If the value is NaN or infinity, different versions of libc
  // produce different outputs (some will format it signed and some won't). To
  // get reliable output, handle it ourselves.
  if (isnan(value)) return OBJ_VAL(copy_string("nan", 3));
  if (isinf(value))
  {
    if (value > 0.0)
    {
      return OBJ_VAL(copy_string("infinity", 8));
    }
    else
    {
	  return OBJ_VAL(copy_string("-infinity", 9));
    }
  }

  // This is large enough to hold any double converted to a string using
  // "%.14g". Example:
  //
  //     -1.12345678901234e-1022
  //
  // So we have:
  //
  // + 1 char for sign
  // + 1 char for digit
  // + 1 char for "."
  // + 14 chars for decimal digits
  // + 1 char for "e"
  // + 1 char for "-" or "+"
  // + 4 chars for exponent
  // + 1 char for "\0"
  // = 24
  char buffer[24];
  int length = sprintf(buffer, "%.14g", value);
  return OBJ_VAL(copy_string(buffer, length));
}

DEF_PRIMITIVE(num_toString)
{
	RETURN_VAL(num_to_string(AS_NUMBER(args[0])));
}

DEF_PRIMITIVE(list_new)
{
	RETURN_OBJ(new_list(0));
}

DEF_PRIMITIVE(list_filled)
{
	if (!validate_int(args[1], "Size")) {
		return false;
	}
	if (AS_NUMBER(args[1]) < 0) {
		RETURN_ERROR("Size cannot be negative.");
	}

	uint32_t size = (uint32_t)AS_NUMBER(args[1]);
	ObjList* list = new_list(size);

	for (uint32_t i = 0; i < size; i++) {
		list->elements.values[i] = args[2];
	}

	RETURN_OBJ(list);
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

DEF_PRIMITIVE(list_isEmpty)
{
	RETURN_BOOL(AS_LIST(args[0])->elements.count == 0);
}

DEF_PRIMITIVE(list_iterate)
{
	ObjList* list = AS_LIST(args[0]);

	if (IS_NIL(args[1]))
	{
		if (list->elements.count == 0) {
			RETURN_FALSE;
		}
		RETURN_NUM(0);
	}

	if (!validate_int(args[1], "Iterator")) {
		return false;
	}

	double index = AS_NUMBER(args[1]);
	if (index < 0 || index >= list->elements.count - 1) {
		RETURN_FALSE;
	}

	RETURN_NUM(index + 1);
}

DEF_PRIMITIVE(list_iteratorValue)
{
	ObjList* list = AS_LIST(args[0]);
	uint32_t index = validate_index(args[1], list->elements.count, "Iterator");
	if (index == UINT32_MAX) {
		return false;
	}

	RETURN_VAL(list->elements.values[index]);
}

DEF_PRIMITIVE(list_toString)
{
	ObjList* list = AS_LIST(args[0]);
	RETURN_VAL(OBJ_VAL(copy_string("List", 4)));
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

	ObjString* str = AS_STRING(args[1]);

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

		if (index > map->entries.capacity) {
			RETURN_FALSE;
		}

		// Advance the iterator.
		index++;
	}

	// Find a used entry, if any.
	for (; index <= map->entries.capacity; index++)
	{
		if (map->entries.entries[index].key) {
			RETURN_NUM(index);
		}
	}

	// If we get here, walked all of the entries.
	RETURN_FALSE;
}

DEF_PRIMITIVE(map_iteratorValue)
{
  ObjMap* map = AS_MAP(args[0]);
  uint32_t index = validate_index(args[1], map->entries.capacity + 1, "Iterator");
  if (index == UINT32_MAX) {
	  return false;
  }

  //RETURN_VAL(map->entries.entries[index].value);
  RETURN_VAL(OBJ_VAL(map->entries.entries[index].key));
}

DEF_PRIMITIVE(range_new)
{
	if (!IS_NUMBER(args[-1])) {
		return false;
	}

	ObjRange* range = new_range();
	range->from = AS_NUMBER(args[-1]);

	pop();

	args[-1] = OBJ_VAL(range);
	return true;
}

DEF_PRIMITIVE(range_from)
{
	RETURN_NUM(AS_RANGE(args[0])->from);
}

DEF_PRIMITIVE(range_to)
{
	RETURN_NUM(AS_RANGE(args[0])->to);
}

DEF_PRIMITIVE(range_setTo)
{
	if (!IS_NUMBER(args[1])) {
		return false;
	}

	AS_RANGE(args[0])->to = AS_NUMBER(args[1]);

	RETURN_VAL(args[0]);
}

DEF_PRIMITIVE(range_setInclusive)
{
	if (!IS_BOOL(args[1])) {
		return false;
	}

	AS_RANGE(args[0])->is_inclusive = AS_BOOL(args[1]);

	RETURN_VAL(args[0]);
}

DEF_PRIMITIVE(range_iterate)
{
	ObjRange* range = AS_RANGE(args[0]);

	// Special case: empty range.
	if (range->from == range->to && !range->is_inclusive) {
		RETURN_FALSE;
	}

	// Start the iteration.
	if (IS_NIL(args[1])) {
		RETURN_NUM(range->from);
	}

	if (!IS_NUMBER(args[1])) {
		RETURN_FALSE;
	}

	double iterator = AS_NUMBER(args[1]);

	// Iterate towards [to] from [from].
	if (range->from < range->to)
	{
		iterator++;
		if (iterator > range->to) {
			RETURN_FALSE;
		}
	}
	else
	{
		iterator--;
		if (iterator < range->to) {
			RETURN_FALSE;
		}
	}

	if (!range->is_inclusive && iterator == range->to) {
		RETURN_FALSE;
	}

	RETURN_NUM(iterator);
}

DEF_PRIMITIVE(range_iteratorValue)
{
	// Assume the iterator is a number so that is the value of the range.
	RETURN_VAL(args[1]);
}

static ObjClass* define_class(ObjModule* module, const char* name)
{
	ObjString* name_string = copy_string(name, strlen(name));
	push(OBJ_VAL(name_string));

	ObjClass* class_obj = new_single_class(0, name_string);

	DefineVariable(module, name, name_string->length, OBJ_VAL(class_obj), NULL);

	pop();
	return class_obj;
}

static Value find_variable(ObjModule* module, const char* name)
{
	int symbol = symbol_table_find(&module->variable_names, name, strlen(name));
	if (symbol == -1) {
		return NIL_VAL;
	}
	return module->variables.values[symbol];
}

void initialize_core()
{
	ObjString* core_name = copy_string("Core", 4);

	ObjModule* core_module = new_module(core_name);

	push(OBJ_VAL(core_module));
	table_set(&vm.modules, core_name, OBJ_VAL(core_module));
	pop();

	vm.object_class = define_class(core_module, "Object");
	PRIMITIVE(vm.object_class, "is(_)", object_is);
	PRIMITIVE(vm.object_class, "type", object_type);

	vm.class_class = define_class(core_module, "Class");
	bind_superclass(vm.class_class, vm.object_class);

	vessel_interpret("Core", coreModuleSource);

	vm.bool_class = AS_CLASS(find_variable(core_module, "Bool"));
	//PRIMITIVE(vm->boolClass, "toString", bool_toString);
	//PRIMITIVE(vm->boolClass, "!", bool_not);

	vm.num_class = AS_CLASS(find_variable(core_module, "Num"));
	PRIMITIVE(vm.num_class, "toString", num_toString);

	vm.list_class = AS_CLASS(find_variable(core_module, "List"));
	PRIMITIVE(vm.list_class->obj.class_obj, "new()", list_new);
	PRIMITIVE(vm.list_class->obj.class_obj, "filled(_,_)", list_filled);
	PRIMITIVE(vm.list_class, "[_]", list_subscript);
	PRIMITIVE(vm.list_class, "[_]=(_)", list_subscriptSetter);
	PRIMITIVE(vm.list_class, "add(_)", list_add);
	PRIMITIVE(vm.list_class, "addCore_(_)", list_addCore);
	PRIMITIVE(vm.list_class, "clear()", list_clear);
	PRIMITIVE(vm.list_class, "count", list_count);
	PRIMITIVE(vm.list_class, "removeAt(_)", list_removeAt);
	PRIMITIVE(vm.list_class, "isEmpty", list_isEmpty);
	PRIMITIVE(vm.list_class, "iterate(_)", list_iterate);
	PRIMITIVE(vm.list_class, "iteratorValue(_)", list_iteratorValue);
	PRIMITIVE(vm.list_class, "toString", list_toString);

	vm.map_class = AS_CLASS(find_variable(core_module, "Map"));
	PRIMITIVE(vm.map_class->obj.class_obj, "new()", map_new);
	PRIMITIVE(vm.map_class, "[_]", map_subscript);
	PRIMITIVE(vm.map_class, "[_]=(_)", map_subscriptSetter);
	PRIMITIVE(vm.map_class, "addCore_(_,_)", map_addCore);
	PRIMITIVE(vm.map_class, "clear()", map_clear);
	PRIMITIVE(vm.map_class, "containsKey(_)", map_containsKey);
	PRIMITIVE(vm.map_class, "count", map_count);
	PRIMITIVE(vm.map_class, "remove(_)", map_remove);
	PRIMITIVE(vm.map_class, "iterate(_)", map_iterate);
	PRIMITIVE(vm.map_class, "iteratorValue(_)", map_iteratorValue);

	vm.range_class = AS_CLASS(find_variable(core_module, "Range"));
	DefineVariable(core_module, "Range", 5, OBJ_VAL(vm.range_class), NULL);
	PRIMITIVE(vm.range_class->obj.class_obj, "new()", range_new);
	PRIMITIVE(vm.range_class, "from", range_from);
	PRIMITIVE(vm.range_class, "to", range_to);
	PRIMITIVE(vm.range_class, "setTo(_)", range_setTo);
	PRIMITIVE(vm.range_class, "setInclusive(_)", range_setInclusive);
	PRIMITIVE(vm.range_class, "iterate(_)", range_iterate);
	PRIMITIVE(vm.range_class, "iteratorValue(_)", range_iteratorValue);
}