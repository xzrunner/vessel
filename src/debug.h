#ifdef __cplusplus
extern "C"
{
#endif

#ifndef vessel_debug_h
#define vessel_debug_h

#include "value.h"

void ves_dump_value(Value value);

void ves_str_buf_newline();
void ves_str_buf_clear();
const char* ves_get_str_buf();

#endif // vessel_debug_h

#ifdef __cplusplus
}
#endif