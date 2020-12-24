#ifndef vessel_utils_h
#define vessel_utils_h

#include "value.h"

#include <stdint.h>

int powerof2ceil(int n);

int symbol_table_find(const ValueArray* symbols, const char* name, size_t length);
int symbol_table_ensure(ValueArray* symbols, const char* name, size_t length);
int symbol_table_add(ValueArray* symbols, const char* name, size_t length);

// $ - A C string.
// @ - A Wren string object.
Value string_format(const char* format, ...);

#endif // vessel_utils_h
