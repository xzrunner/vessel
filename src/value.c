#include "value.h"
#include "memory.h"
#include "object.h"
#include "vm.h"

#include <stddef.h>
#include <stdio.h>

#define GROW_FACTOR 2

bool values_equal(Value a, Value b)
{
#ifdef NAN_BOXING
    if (IS_NUMBER(a) && IS_NUMBER(b)) {
        return AS_NUMBER(a) == AS_NUMBER(b);
    }
    return a == b;
#else
    if (a.type != b.type) {
        return false;
    }

    switch (a.type)
    {
    case VAL_BOOL:   return AS_BOOL(a) == AS_BOOL(b);
    case VAL_NIL:    return true;
    case VAL_NUMBER: return AS_NUMBER(a) == AS_NUMBER(b);
    case VAL_OBJ:    return AS_OBJ(a) == AS_OBJ(b);
    default:
        return false; // Unreachable.
    }
#endif
}

void init_value_array(ValueArray* array)
{
    array->values = NULL;
    array->capacity = 0;
    array->count = 0;
}

void write_value_array(ValueArray* array, Value value)
{
    if (array->capacity < array->count + 1)
    {
        int old_cap = array->capacity;
        array->capacity = GROW_CAPACITY(old_cap);
        array->values = GROW_ARRAY(Value, array->values, old_cap, array->capacity);
    }

    array->values[array->count] = value;
    array->count++;
}

void free_value_array(ValueArray* array)
{
    FREE_ARRAY(Value, array->values, array->capacity);
    init_value_array(array);
}

Value array_remove_at(ValueArray* array, uint32_t index)
{
    Value removed = array->values[index];

    if (IS_OBJ(removed)) {
        push(removed);
    }

    for (int i = index; i < array->count - 1; i++) {
        array->values[i] = array->values[i + 1];
    }

    if (array->capacity / GROW_FACTOR >= array->count)
    {
        array->values = (Value*)reallocate(
            array->values,
            sizeof(Value) * array->capacity,
            sizeof(Value) * (array->capacity / GROW_FACTOR)
        );
        array->capacity /= GROW_FACTOR;
    }

    if (IS_OBJ(removed)) {
        pop();
    }

    array->count--;
    return removed;
}