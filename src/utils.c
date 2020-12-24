#include "utils.h"
#include "object.h"
#include "value.h"
#include "vm.h"
#include "common.h"
#include "memory.h"

#include <stdarg.h>
#include <stdio.h>

// From: http://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2Float
int powerof2ceil(int n)
{
	n--;
	n |= n >> 1;
	n |= n >> 2;
	n |= n >> 4;
	n |= n >> 8;
	n |= n >> 16;
	n++;

	return n;
}

int symbol_table_find(const ValueArray* symbols, const char* name, size_t length)
{
	for (int i = 0; i < symbols->count; i++)
	{
		const Value* v = &symbols->values[i];
		ASSERT(IS_STRING(*v), "not string");

		ObjString* str = AS_STRING(*v);
		if (str->length == length && memcmp(str->chars, name, length) == 0) {
			return i;
		}
	}
	return -1;
}

int symbol_table_ensure(ValueArray* symbols, const char* name, size_t length)
{
	int existing = symbol_table_find(symbols, name, length);
	if (existing != -1) {
		return existing;
	}

	write_value_array(symbols, OBJ_VAL(copy_string(name, length)));
	return symbols->count - 1;
}

int symbol_table_add(ValueArray* symbols, const char* name, size_t length)
{
    ObjString* symbol = copy_string(name, length);

    push(OBJ_VAL(symbol));
    write_value_array(symbols, OBJ_VAL(symbol));
    pop();

    return symbols->count - 1;
}

Value string_format(const char* format, ...)
{
    va_list arg_list;

    // Calculate the length of the result string. Do this up front so we can
    // create the final string with a single allocation.
    va_start(arg_list, format);
    size_t total_length = 0;
    for (const char* c = format; *c != '\0'; c++)
    {
        switch (*c)
        {
        case '$':
            total_length += strlen(va_arg(arg_list, const char*));
            break;

        case '@':
            total_length += AS_STRING(va_arg(arg_list, Value))->length;
            break;

        default:
            // Any other character is interpreted literally.
            total_length++;
        }
    }
    va_end(arg_list);

    // Concatenate the string.
    char* heap_chars = ALLOCATE(char, total_length + 1);
    ObjString* result = allocate_string(heap_chars, total_length, 0);

    va_start(arg_list, format);
    char* start = result->chars;
    for (const char* c = format; *c != '\0'; c++)
    {
        switch (*c)
        {
        case '$':
        {
            const char* string = va_arg(arg_list, const char*);
            size_t length = strlen(string);
            memcpy(start, string, length);
            start += length;
            break;
        }

        case '@':
        {
            ObjString* string = AS_STRING(va_arg(arg_list, Value));
            memcpy(start, string->chars, string->length);
            start += string->length;
            break;
        }

        default:
            // Any other character is interpreted literally.
            *start++ = *c;
        }
    }
    va_end(arg_list);

    result->hash = hash_string(result->chars, start - result->chars);

    return OBJ_VAL(result);
}