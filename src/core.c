#include "core.h"
#include "object.h"
#include "vm.h"
#include "primitive.h"
#include "utils.h"
#include "core.ves.inc"
#include "debug.h"
#include "memory.h"

#include <math.h>
#include <stdio.h>
#include <time.h>

DEF_PRIMITIVE(w_Object_is)
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

DEF_PRIMITIVE(w_Object_type)
{
	RETURN_OBJ(get_class(args[0]));
}

DEF_PRIMITIVE(w_Object_toString)
{
	RETURN_VAL(string_format("instance of @", OBJ_VAL(get_class(args[0])->name)));
}

DEF_PRIMITIVE(w_Class_name)
{
	RETURN_OBJ(AS_CLASS(args[0])->name);
}

DEF_PRIMITIVE(w_Class_toString)
{
	RETURN_OBJ(AS_CLASS(args[0])->name);
}

DEF_PRIMITIVE(w_Bool_toString)
{
	bool b = AS_BOOL(args[0]);
	if (b) {
		RETURN_VAL(OBJ_VAL(copy_string("true", 4)));
	} else {
		RETURN_VAL(OBJ_VAL(copy_string("false", 5)));
	}
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

DEF_PRIMITIVE(w_Num_toString)
{
	RETURN_VAL(num_to_string(AS_NUMBER(args[0])));
}

DEF_PRIMITIVE(w_Null_not)
{
	RETURN_VAL(TRUE_VAL);
}

DEF_PRIMITIVE(w_Null_toString)
{
	RETURN_VAL(OBJ_VAL(copy_string("null", 4)));
}

DEF_PRIMITIVE(w_String_count)
{
	RETURN_NUM(AS_STRING(args[0])->length);
}

// https://stackoverflow.com/questions/779875/what-function-is-to-replace-a-substring-from-a-string-in-c
DEF_PRIMITIVE(w_String_replace)
{
	if (!IS_STRING(args[0]) ||
		!IS_STRING(args[1]) ||
		!IS_STRING(args[2])) {
		return false;
	}

	ObjString* orig = AS_STRING(args[0]);
	ObjString* rep = AS_STRING(args[1]);
	ObjString* with = AS_STRING(args[2]);

	if (rep->length == 0) {
		return true;
	}

    char* ins = orig->chars;
	char* tmp = NULL;
	int count = 0;
    for ( ; tmp = strstr(ins, rep->chars); ++count) {
        ins = tmp + rep->length;
    }

	int len = orig->length + (with->length - rep->length) * count;
	tmp = ALLOCATE(char, orig->length + (with->length - rep->length) * count + 1);
	if (!tmp) {
		return false;
	}

	char* p_orig = orig->chars;
	char* p_tmp = tmp;
    while (count--) 
	{
        ins = strstr(p_orig, rep->chars);
        int len_front = ins - p_orig;
        p_tmp = strncpy(p_tmp, p_orig, len_front) + len_front;
        p_tmp = strcpy(p_tmp, with->chars) + with->length;
		p_orig += len_front + rep->length;
    }
    strcpy(p_tmp, p_orig);

	RETURN_VAL(OBJ_VAL(take_string(tmp, len)));
}

DEF_PRIMITIVE(w_String_toString)
{
	RETURN_VAL(args[0]);
}

DEF_PRIMITIVE(w_List_new)
{
	RETURN_OBJ(new_list(0));
}

DEF_PRIMITIVE(w_List_filled)
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

DEF_PRIMITIVE(w_List_subscript)
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

DEF_PRIMITIVE(w_List_subscriptSetter)
{
	ObjList* list = AS_LIST(args[0]);

	uint32_t index = validate_index(args[1], list->elements.count, "Subscript");
	if (index == UINT32_MAX) {
		return false;
	}

	list->elements.values[index] = args[2];
	RETURN_VAL(args[2]);
}

DEF_PRIMITIVE(w_List_add)
{
	write_value_array(&AS_LIST(args[0])->elements, args[1]);
	RETURN_VAL(args[1]);
}

DEF_PRIMITIVE(w_List_addCore)
{
	write_value_array(&AS_LIST(args[0])->elements, args[1]);

	// Return the list.
	RETURN_VAL(args[0]);
}

DEF_PRIMITIVE(w_List_clear)
{
	free_value_array(&AS_LIST(args[0])->elements);
	RETURN_NULL;
}

DEF_PRIMITIVE(w_List_count)
{
	RETURN_NUM(AS_LIST(args[0])->elements.count);
}

DEF_PRIMITIVE(w_List_removeAt)
{
	ObjList* list = AS_LIST(args[0]);
	uint32_t index = validate_index(args[1], list->elements.count, "Index");
	if (index == UINT32_MAX) {
		return false;
	}

	RETURN_VAL(array_remove_at(&list->elements, index));
}

DEF_PRIMITIVE(w_List_isEmpty)
{
	RETURN_BOOL(AS_LIST(args[0])->elements.count == 0);
}

DEF_PRIMITIVE(w_List_iterate)
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

DEF_PRIMITIVE(w_List_iteratorValue)
{
	ObjList* list = AS_LIST(args[0]);
	uint32_t index = validate_index(args[1], list->elements.count, "Iterator");
	if (index == UINT32_MAX) {
		return false;
	}

	RETURN_VAL(list->elements.values[index]);
}

DEF_PRIMITIVE(w_Map_new)
{
  RETURN_OBJ(new_map());
}

DEF_PRIMITIVE(w_Map_subscript)
{
	//if (!validate_key(args[1])) {
	//	return false;
	//}
	if (!IS_STRING(args[1])) {
		return false;
	}

	ObjString* str = AS_STRING(args[1]);

	Value value = NIL_VAL;
	table_get(&AS_MAP(args[0])->entries, AS_STRING(args[1]), &value);

	RETURN_VAL(value);
}

DEF_PRIMITIVE(w_Map_subscriptSetter)
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
DEF_PRIMITIVE(w_Map_addCore)
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

DEF_PRIMITIVE(w_Map_clear)
{
	free_table(&AS_MAP(args[0])->entries);
	RETURN_NULL;
}

DEF_PRIMITIVE(w_Map_containsKey)
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

DEF_PRIMITIVE(w_Map_count)
{
	RETURN_NUM(AS_MAP(args[0])->entries.count);
}

DEF_PRIMITIVE(w_Map_remove)
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

DEF_PRIMITIVE(w_Map_iterate)
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

DEF_PRIMITIVE(w_Map_keyIteratorValue)
{
	ObjMap* map = AS_MAP(args[0]);
	uint32_t index = validate_index(args[1], map->entries.capacity + 1, "Iterator");
	if (index == UINT32_MAX) {
		return false;
	}

	RETURN_VAL(OBJ_VAL(map->entries.entries[index].key));
}

DEF_PRIMITIVE(w_Map_valueIteratorValue)
{
	ObjMap* map = AS_MAP(args[0]);
	uint32_t index = validate_index(args[1], map->entries.capacity + 1, "Iterator");
	if (index == UINT32_MAX) {
		return false;
	}

	RETURN_VAL(map->entries.entries[index].value);
}


DEF_PRIMITIVE(w_Set_new)
{
	RETURN_OBJ(new_set());
}

DEF_PRIMITIVE(w_Set_add)
{
	ObjSet* set = AS_SET(args[0]);
	for (int i = 0; i < set->elements.count; ++i) {
		if (values_equal(args[1], set->elements.values[i])) {
			RETURN_FALSE;
		}
	}

	write_value_array(&set->elements, args[1]);
	RETURN_VAL(args[1]);
}

DEF_PRIMITIVE(w_Set_clear)
{
	free_value_array(&AS_SET(args[0])->elements);
	RETURN_NULL;
}

DEF_PRIMITIVE(w_Set_count)
{
	RETURN_NUM(AS_SET(args[0])->elements.count);
}

DEF_PRIMITIVE(w_Set_remove)
{
	ObjSet* set = AS_SET(args[0]);

	for (int i = 0; i < set->elements.count; ++i) {
		if (values_equal(args[1], set->elements.values[i])) {
			RETURN_VAL(array_remove_at(&set->elements, i));
		}
	}
	RETURN_NULL;
}

DEF_PRIMITIVE(w_Set_isEmpty)
{
	RETURN_BOOL(AS_SET(args[0])->elements.count == 0);
}

DEF_PRIMITIVE(w_Set_front)
{
	ObjSet* set = AS_SET(args[0]);
	if (set->elements.count > 0) {
		RETURN_VAL(set->elements.values[0]);
	} else {
		RETURN_NULL;
	}
}

DEF_PRIMITIVE(w_Set_iterate)
{
	ObjSet* set = AS_SET(args[0]);

	if (IS_NIL(args[1]))
	{
		if (set->elements.count == 0) {
			RETURN_FALSE;
		}
		RETURN_NUM(0);
	}

	if (!validate_int(args[1], "Iterator")) {
		return false;
	}

	double index = AS_NUMBER(args[1]);
	if (index < 0 || index >= set->elements.count - 1) {
		RETURN_FALSE;
	}

	RETURN_NUM(index + 1);
}

DEF_PRIMITIVE(w_Set_iteratorValue)
{
	ObjSet* set = AS_SET(args[0]);
	uint32_t index = validate_index(args[1], set->elements.count, "Iterator");
	if (index == UINT32_MAX) {
		return false;
	}

	RETURN_VAL(set->elements.values[index]);
}

DEF_PRIMITIVE(w_Range_new)
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

DEF_PRIMITIVE(w_Range_from)
{
	RETURN_NUM(AS_RANGE(args[0])->from);
}

DEF_PRIMITIVE(w_Range_to)
{
	RETURN_NUM(AS_RANGE(args[0])->to);
}

DEF_PRIMITIVE(w_Range_setTo)
{
	if (!IS_NUMBER(args[1])) {
		return false;
	}

	AS_RANGE(args[0])->to = AS_NUMBER(args[1]);

	RETURN_VAL(args[0]);
}

DEF_PRIMITIVE(w_Range_setInclusive)
{
	if (!IS_BOOL(args[1])) {
		return false;
	}

	AS_RANGE(args[0])->is_inclusive = AS_BOOL(args[1]);

	RETURN_VAL(args[0]);
}

DEF_PRIMITIVE(w_Range_iterate)
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

DEF_PRIMITIVE(w_Range_iteratorValue)
{
	// Assume the iterator is a number so that is the value of the range.
	RETURN_VAL(args[1]);
}

DEF_PRIMITIVE(w_System_writeString)
{
	dump_value(args[1], false);
	return true;
}

DEF_PRIMITIVE(w_System_clock)
{
	RETURN_NUM((double)clock() / CLOCKS_PER_SEC);
}

DEF_PRIMITIVE(w_Basic_loadstring)
{
	ObjString* str = AS_STRING(args[1]);
	if (str->length > 0) {
		ves_interpret("temp", str->chars);
	}
	return true;
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
	PRIMITIVE(vm.object_class, "is(_)", w_Object_is);
	PRIMITIVE(vm.object_class, "type", w_Object_type);
	PRIMITIVE(vm.object_class, "toString()", w_Object_toString);

	vm.class_class = define_class(core_module, "Class");
	PRIMITIVE(vm.class_class, "name", w_Class_name);
	PRIMITIVE(vm.class_class, "toString()", w_Class_toString);
	bind_superclass(vm.class_class, vm.object_class);

	ves_interpret("Core", coreModuleSource);

	vm.bool_class = AS_CLASS(find_variable(core_module, "Bool"));
	PRIMITIVE(vm.bool_class, "toString()", w_Bool_toString);
	//PRIMITIVE(vm.bool_class, "!", bool_not);

	vm.num_class = AS_CLASS(find_variable(core_module, "Num"));
	PRIMITIVE(vm.num_class, "toString()", w_Num_toString);

	vm.null_class = AS_CLASS(find_variable(core_module, "Null"));
	PRIMITIVE(vm.null_class, "!", w_Null_not);
	PRIMITIVE(vm.null_class, "toString()", w_Null_toString);

	vm.string_class = AS_CLASS(find_variable(core_module, "String"));
	PRIMITIVE(vm.string_class, "count", w_String_count);
	PRIMITIVE(vm.string_class, "replace(_,_)", w_String_replace);
	PRIMITIVE(vm.string_class, "toString()", w_String_toString);
	for (int i = 0; i < vm.strings.capacity; ++i) {
		if (vm.strings.entries[i].key) {
			vm.strings.entries[i].key->obj.class_obj = vm.string_class;
		}
	}

	vm.list_class = AS_CLASS(find_variable(core_module, "List"));
	PRIMITIVE(vm.list_class->obj.class_obj, "new()", w_List_new);
	PRIMITIVE(vm.list_class->obj.class_obj, "filled(_,_)", w_List_filled);
	PRIMITIVE(vm.list_class, "[_]", w_List_subscript);
	PRIMITIVE(vm.list_class, "[_]=(_)", w_List_subscriptSetter);
	PRIMITIVE(vm.list_class, "add(_)", w_List_add);
	PRIMITIVE(vm.list_class, "addCore_(_)", w_List_addCore);
	PRIMITIVE(vm.list_class, "clear()", w_List_clear);
	PRIMITIVE(vm.list_class, "count", w_List_count);
	PRIMITIVE(vm.list_class, "removeAt(_)", w_List_removeAt);
	PRIMITIVE(vm.list_class, "isEmpty", w_List_isEmpty);
	PRIMITIVE(vm.list_class, "iterate(_)", w_List_iterate);
	PRIMITIVE(vm.list_class, "iteratorValue(_)", w_List_iteratorValue);

	vm.map_class = AS_CLASS(find_variable(core_module, "Map"));
	PRIMITIVE(vm.map_class->obj.class_obj, "new()", w_Map_new);
	PRIMITIVE(vm.map_class, "[_]", w_Map_subscript);
	PRIMITIVE(vm.map_class, "[_]=(_)", w_Map_subscriptSetter);
	PRIMITIVE(vm.map_class, "addCore_(_,_)", w_Map_addCore);
	PRIMITIVE(vm.map_class, "clear()", w_Map_clear);
	PRIMITIVE(vm.map_class, "containsKey(_)", w_Map_containsKey);
	PRIMITIVE(vm.map_class, "count", w_Map_count);
	PRIMITIVE(vm.map_class, "remove(_)", w_Map_remove);
	PRIMITIVE(vm.map_class, "iterate(_)", w_Map_iterate);
	PRIMITIVE(vm.map_class, "keyIteratorValue_(_)", w_Map_keyIteratorValue);
	PRIMITIVE(vm.map_class, "valueIteratorValue_(_)", w_Map_valueIteratorValue);

	vm.set_class = AS_CLASS(find_variable(core_module, "Set"));
	PRIMITIVE(vm.set_class->obj.class_obj, "new()", w_Set_new);
	PRIMITIVE(vm.set_class, "add(_)", w_Set_add);
	PRIMITIVE(vm.set_class, "clear()", w_Set_clear);
	PRIMITIVE(vm.set_class, "count", w_Set_count);
	PRIMITIVE(vm.set_class, "remove(_)", w_Set_remove);
	PRIMITIVE(vm.set_class, "isEmpty", w_Set_isEmpty);
	PRIMITIVE(vm.set_class, "front()", w_Set_front);
	PRIMITIVE(vm.set_class, "iterate(_)", w_Set_iterate);
	PRIMITIVE(vm.set_class, "iteratorValue(_)", w_Set_iteratorValue);

	vm.range_class = AS_CLASS(find_variable(core_module, "Range"));
	DefineVariable(core_module, "Range", 5, OBJ_VAL(vm.range_class), NULL);
	PRIMITIVE(vm.range_class->obj.class_obj, "new()", w_Range_new);
	PRIMITIVE(vm.range_class, "from", w_Range_from);
	PRIMITIVE(vm.range_class, "to", w_Range_to);
	PRIMITIVE(vm.range_class, "setTo(_)", w_Range_setTo);
	PRIMITIVE(vm.range_class, "setInclusive(_)", w_Range_setInclusive);
	PRIMITIVE(vm.range_class, "iterate(_)", w_Range_iterate);
	PRIMITIVE(vm.range_class, "iteratorValue(_)", w_Range_iteratorValue);

	vm.system_class = AS_CLASS(find_variable(core_module, "System"));
	DefineVariable(core_module, "System", 6, OBJ_VAL(vm.system_class), NULL);
	PRIMITIVE(vm.system_class->obj.class_obj, "writeString(_)", w_System_writeString);
	PRIMITIVE(vm.system_class->obj.class_obj, "clock()", w_System_clock);

	vm.basic_class = AS_CLASS(find_variable(core_module, "Basic"));
	DefineVariable(core_module, "Basic", 5, OBJ_VAL(vm.basic_class), NULL);
	PRIMITIVE(vm.basic_class->obj.class_obj, "loadstring(_)", w_Basic_loadstring);
}