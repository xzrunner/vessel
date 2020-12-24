#ifndef vessel_primitive_h
#define vessel_primitive_h

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

#endif // vessel_primitive_h