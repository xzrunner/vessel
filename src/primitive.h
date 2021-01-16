#ifndef vessel_primitive_h
#define vessel_primitive_h

#include "value.h"

#include <stdint.h>

#define PRIMITIVE(cls, name, function)                                         \
    do                                                                         \
    {                                                                          \
        ObjString* key = copy_string(name, strlen(name));                      \
        ObjMethod* val = new_method();                                         \
        val->type = METHOD_PRIMITIVE;                                          \
        val->as.primitive = prim_##function;                                   \
        table_set(&cls->methods, key, OBJ_VAL(val));                           \
    } while (false)

#define DEF_PRIMITIVE(name)                                                    \
    static bool prim_##name(Value* args)

#define RETURN_VAL(value)                                                      \
    do                                                                         \
    {                                                                          \
        args[0] = value;                                                       \
        return true;                                                           \
    } while (false)

#define RETURN_OBJ(obj)     RETURN_VAL(OBJ_VAL(obj))
#define RETURN_BOOL(value)  RETURN_VAL(BOOL_VAL(value))
#define RETURN_NULL         RETURN_VAL(NIL_VAL)
#define RETURN_NUM(value)   RETURN_VAL(NUMBER_VAL(value))
#define RETURN_FALSE        RETURN_VAL(FALSE_VAL)

#define RETURN_ERROR(msg)                                                      \
    do                                                                         \
    {                                                                          \
      vm.error = OBJ_VAL(copy_string(msg, sizeof(msg) - 1));                   \
      return false;                                                            \
    } while (false)

#define RETURN_ERROR_FMT(...)                                                  \
    do                                                                         \
    {                                                                          \
      vm.error = string_format(__VA_ARGS__);                                   \
      return false;                                                            \
    } while (false)

bool validate_num(Value arg, const char* arg_name);
bool validate_int_value(double value, const char* arg_name);
bool validate_int(Value arg, const char* arg_name);
bool validate_key(Value arg);
uint32_t validate_index(Value arg, uint32_t count, const char* arg_name);

#endif // vessel_primitive_h
