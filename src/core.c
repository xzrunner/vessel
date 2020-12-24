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

//DEF_PRIMITIVE(list_subscript)
//{
//	ObjList* list = AS_LIST(args[0]);
//
//	if (IS_NUM(args[1]))
//	{
//		uint32_t index = validateIndex(vm, args[1], list->elements.count,
//										"Subscript");
//		if (index == UINT32_MAX) return false;
//
//		RETURN_VAL(list->elements.data[index]);
//	}
//
//	if (!IS_RANGE(args[1]))
//	{
//		RETURN_ERROR("Subscript must be a number or a range.");
//	}
//
//	int step;
//	uint32_t count = list->elements.count;
//	uint32_t start = calculateRange(vm, AS_RANGE(args[1]), &count, &step);
//	if (start == UINT32_MAX) return false;
//
//	ObjList* result = wrenNewList(vm, count);
//	for (uint32_t i = 0; i < count; i++)
//	{
//		result->elements.data[i] = list->elements.data[start + i * step];
//	}
//
//	RETURN_OBJ(result);
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

	//vm.list_class = define_class(core_module, "List");
	vm.list_class = new_class(copy_string("List", 4));
	define_variable(core_module, "List", 4, OBJ_VAL(vm.list_class));
	//PRIMITIVE(vm.list_class->obj.class_obj, "filled(_,_)", list_filled);
	PRIMITIVE(vm.list_class->obj.class_obj, "new()", list_new);
	//PRIMITIVE(vm.list_class, "[_]", list_subscript);
	//PRIMITIVE(vm.list_class, "[_]=(_)", list_subscriptSetter);
	PRIMITIVE(vm.list_class, "add(_)", list_add);
	PRIMITIVE(vm.list_class, "addCore_(_)", list_addCore);
	//PRIMITIVE(vm.list_class, "clear()", list_clear);
	//PRIMITIVE(vm.list_class, "count", list_count);
	//PRIMITIVE(vm.list_class, "insert(_,_)", list_insert);
	//PRIMITIVE(vm.list_class, "iterate(_)", list_iterate);
	//PRIMITIVE(vm.list_class, "iteratorValue(_)", list_iteratorValue);
	//PRIMITIVE(vm.list_class, "removeAt(_)", list_removeAt);
	//PRIMITIVE(vm.list_class, "indexOf(_)", list_indexOf);
	//PRIMITIVE(vm.list_class, "swap(_,_)", list_swap);
}