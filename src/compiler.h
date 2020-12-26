#ifndef vessel_compiler_h
#define vessel_compiler_h

#include "common.h"
#include "object.h"

ObjFunction* compile(const char* module, const char* source);
void mark_compiler_roots();

typedef enum
{
    SIG_METHOD,
    SIG_GETTER,
    SIG_SETTER,
    SIG_SUBSCRIPT,
    SIG_SUBSCRIPT_SETTER,
    SIG_INITIALIZER
} SignatureType;

typedef struct
{
    const char* name;
    int length;
    SignatureType type;
    int arity;
} Signature;

void signature_to_string(Signature* signature, char name[MAX_METHOD_SIGNATURE], int* length);

#endif // vessel_compiler_h
