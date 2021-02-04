#include "debug.h"
#include "object.h"
#include "memory.h"
#include "vm.h"

#include <stdio.h>
#include <stdarg.h>

static char OUT_BUF[1024];

static void print(bool to_console, const char* format, ...)
{
	va_list args;
	va_start(args, format);

	if (to_console)
	{
		vprintf(format, args);
	}
	else
	{
		vsprintf(&OUT_BUF[0], format, args);
		if (vm.config.write_fn != NULL) {
			vm.config.write_fn(OUT_BUF);
		}
	}

	va_end(args);
}

static void dump_function(ObjFunction* function, bool to_console)
{
	if (function->name == NULL) {
		print(to_console, "<script>");
		return;
	}

	print(to_console, "<fn %s>", function->name->chars);
}

static void dump_object(Value value, bool to_console)
{
	switch (OBJ_TYPE(value))
	{
	case OBJ_CLASS:
		print(to_console, "%s", AS_CLASS(value)->name->chars);
		break;
	case OBJ_BOUND_METHOD:
		dump_function(AS_BOUND_METHOD(value)->method->function, to_console);
		break;
	case OBJ_CLOSURE:
		dump_function(AS_CLOSURE(value)->function, to_console);
		break;
	case OBJ_METHOD:
		print(to_console, "method");
		break;
	case OBJ_FUNCTION:
		dump_function(AS_FUNCTION(value), to_console);
		break;
	case OBJ_FOREIGN:
		print(to_console, "foreign");
		break;
	case OBJ_INSTANCE:
		print(to_console, "%s instance", AS_INSTANCE(value)->klass->name->chars);
		break;
	case OBJ_NATIVE:
		print(to_console, "<native fn>");
		break;
		break;
	case OBJ_STRING:
		print(to_console, "%s", AS_CSTRING(value));
		break;
	case OBJ_UPVALUE:
		print(to_console, "upvalue");
		break;
	case OBJ_MODULE:
		print(to_console, "%s module", AS_MODULE(value)->name->chars);
		break;
	case OBJ_LIST:
	{
		ObjList* list = AS_LIST(value);
		print(to_console, "[");
		for (int i = 0; i < list->elements.count; ++i) {
			dump_value(list->elements.values[i], to_console);
			if (i != list->elements.count - 1) {
				print(to_console, ", ");
			}
		}
		print(to_console, "]");
	}
		break;
	case OBJ_MAP:
		print(to_console, "map");
		break;
	case OBJ_SET:
		print(to_console, "set");
		break;
	case OBJ_RANGE:
	{
		ObjRange* range = AS_RANGE(value);
		print(to_console, "range(%.14g, %.14g)", range->from, range->to);
	}
		break;
	}
}

void dump_value(Value value, bool to_console)
{
#ifdef NAN_BOXING
	if (IS_BOOL(value)) {
		print(to_console, AS_BOOL(value) ? "true" : "false");
	}
	else if (IS_NIL(value)) {
		print(to_console, "nil");
	}
	else if (IS_NUMBER(value)) {
		print(to_console, "%.14g", AS_NUMBER(value));
	}
	else if (IS_OBJ(value)) {
		dump_object(value, to_console);
	}
#else
	switch (value.type)
	{
	case VAL_BOOL:
		print(to_console, AS_BOOL(value) ? "true" : "false");
		break;
	case VAL_NIL:
		print(to_console, "nil");
		break;
	case VAL_NUMBER:
		print(to_console, "%.14g", AS_NUMBER(value));
		break;
	case VAL_OBJ:
		dump_object(value, to_console);
		break;
	}
#endif // NAN_BOXING
}