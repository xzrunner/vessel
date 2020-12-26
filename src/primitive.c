#include "primitive.h"
#include "vm.h"
#include "utils.h"

#include <math.h>

static uint32_t validate_index_value(uint32_t count, double value, const char* arg_name)
{
    if (!validate_int_value(value, arg_name)) {
        return UINT32_MAX;
    }

    if (value < 0) {
        value = count + value;
    }

    if (value >= 0 && value < count) {
        return (uint32_t)value;
    }

    vm.error = string_format("$ out of bounds.", arg_name);
    return UINT32_MAX;
}

bool validate_num(Value arg, const char* arg_name)
{
    if (IS_NUMBER(arg)) {
        return true;
    }
    RETURN_ERROR_FMT("$ must be a number.", arg_name);
}

bool validate_int_value(double value, const char* arg_name)
{
    if (trunc(value) == value) {
        return true;
    }
    RETURN_ERROR_FMT("$ must be an integer.", arg_name);
}

bool validate_int(Value arg, const char* arg_name)
{
    if (!validate_num(arg, arg_name)) {
        return false;
    }
    return validate_int_value(AS_NUMBER(arg), arg_name);
}

bool validate_key(Value arg)
{
    if (IS_BOOL(arg) || IS_CLASS(arg) || IS_NIL(arg) ||
        IS_NUMBER(arg) || IS_STRING(arg))
    {
        return true;
    }

    RETURN_ERROR_FMT("Key must be a value type.");
}

uint32_t validate_index(Value arg, uint32_t count, const char* arg_name)
{
    if (!validate_num(arg, arg_name)) {
        return UINT32_MAX;
    }
    return validate_index_value(count, AS_NUMBER(arg), arg_name);
}