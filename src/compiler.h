#ifdef __cplusplus
extern "C"
{
#endif

#ifndef vessel_compiler_h
#define vessel_compiler_h

#include "common.h"
#include "object.h"

ObjFunction* compile(const char* module, const char* source);
void mark_compiler_roots();

#endif // vessel_compiler_h

#ifdef __cplusplus
}
#endif