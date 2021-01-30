#include "buffer.h"
#include "utils.h"
#include "memory.h"

#define DEFINE_BUFFER(name, type)                                              \
    void name##BufferInit(name##Buffer* buffer)                                \
    {                                                                          \
        buffer->data = NULL;                                                   \
        buffer->capacity = 0;                                                  \
        buffer->count = 0;                                                     \
    }                                                                          \
                                                                               \
    void name##BufferClear(name##Buffer* buffer)                               \
    {                                                                          \
        reallocate(buffer->data, 0, 0);                                        \
        name##BufferInit(buffer);                                              \
    }                                                                          \
                                                                               \
    void name##BufferFill(name##Buffer* buffer, type data, int count)          \
    {                                                                          \
        if (buffer->capacity < buffer->count + count)                          \
        {                                                                      \
            int capacity = powerof2ceil(buffer->count + count);                \
            buffer->data = (type*)reallocate(buffer->data,                     \
                buffer->capacity * sizeof(type), capacity * sizeof(type));     \
            buffer->capacity = capacity;                                       \
        }                                                                      \
                                                                               \
        for (int i = 0; i < count; i++)                                        \
        {                                                                      \
            buffer->data[buffer->count++] = data;                              \
        }                                                                      \
    }                                                                          \
                                                                               \
    void name##BufferWrite(name##Buffer* buffer, type data)                    \
    {                                                                          \
        name##BufferFill(buffer, data, 1);                                     \
    }

DEFINE_BUFFER(Byte, uint8_t);
DEFINE_BUFFER(Int, int);
DEFINE_BUFFER(String, ObjString*);