#include "debug.h"
#include "object.h"
#include "memory.h"

#include <stdio.h>
#include <stdarg.h>

#define OUT_BUF_SIZE 1024

char* out_buf = NULL;
size_t out_ptr = 0;

static void print2buf(const char* format, ...)
{
	va_list args;
	va_start(args, format);
	if (out_buf == NULL) {
		out_buf = ALLOCATE(char, OUT_BUF_SIZE);
		memset(out_buf, 0, OUT_BUF_SIZE);
		out_ptr = 0;
	}

	out_ptr += vsprintf(&out_buf[out_ptr], format, args);

	va_end(args);
}

static void dump_function(ObjFunction* function)
{
	if (function->name == NULL) {
		print2buf("<script>");
		return;
	}

	print2buf("<fn %s>", function->name->chars);
}

static void dump_object(Value value)
{
	switch (OBJ_TYPE(value))
	{
	case OBJ_CLASS:
		print2buf("%s", AS_CLASS(value)->name->chars);
		break;
	case OBJ_BOUND_METHOD:
		dump_function(AS_BOUND_METHOD(value)->method->function);
		break;
	case OBJ_CLOSURE:
		dump_function(AS_CLOSURE(value)->function);
		break;
	case OBJ_METHOD:
		print2buf("method");
		break;
	case OBJ_FUNCTION:
		dump_function(AS_FUNCTION(value));
		break;
	case OBJ_FOREIGN:
		print2buf("foreign");
		break;
	case OBJ_INSTANCE:
		print2buf("%s instance", AS_INSTANCE(value)->klass->name->chars);
		break;
	case OBJ_NATIVE:
		print2buf("<native fn>");
		break;
		break;
	case OBJ_STRING:
		print2buf("%s", AS_CSTRING(value));
		break;
	case OBJ_UPVALUE:
		print2buf("upvalue");
		break;
	case OBJ_MODULE:
		print2buf("%s module", AS_MODULE(value)->name->chars);
		break;
	case OBJ_LIST:
	{
		ObjList* list = AS_LIST(value);
		print2buf("[");
		for (int i = 0; i < list->elements.count; ++i) {
			ves_dump_value(list->elements.values[i]);
			if (i != list->elements.count - 1) {
				print2buf(", ");
			}
		}
		print2buf("]");
	}
		break;
	case OBJ_MAP:
		print2buf("map");
		break;
	}
}

void ves_dump_value(Value value)
{
#ifdef NAN_BOXING
	if (IS_BOOL(value)) {
		print2buf(AS_BOOL(value) ? "true" : "false");
	}
	else if (IS_NIL(value)) {
		print2buf("nil");
	}
	else if (IS_NUMBER(value)) {
		print2buf("%g", AS_NUMBER(value));
	}
	else if (IS_OBJ(value)) {
		dump_object(value);
	}
#else
	switch (value.type)
	{
	case VAL_BOOL:
		print2buf(AS_BOOL(value) ? "true" : "false");
		break;
	case VAL_NIL:
		print2buf("nil");
		break;
	case VAL_NUMBER:
		print2buf("%g", AS_NUMBER(value));
		break;
	case VAL_OBJ:
		dump_object(value);
		break;
	}
#endif // NAN_BOXING
}

void ves_str_buf_newline()
{
	print2buf("\n");
}

void ves_str_buf_clear()
{
	if (out_buf) {
		out_buf[0] = 0;
	}
	out_ptr = 0;
}

const char* ves_get_str_buf()
{
	return out_buf;
}