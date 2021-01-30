#ifndef vessel_buffer_h
#define vessel_buffer_h

#include "value.h"

#define DECLARE_BUFFER(name, type)                                             \
    typedef struct                                                             \
    {                                                                          \
        type* data;                                                            \
        int count;                                                             \
        int capacity;                                                          \
    } name##Buffer;                                                            \
    void name##BufferInit(name##Buffer* buffer);                               \
    void name##BufferClear(name##Buffer* buffer);                              \
    void name##BufferFill(name##Buffer* buffer, type data, int count);         \
    void name##BufferWrite(name##Buffer* buffer, type data)

DECLARE_BUFFER(Byte, uint8_t);
DECLARE_BUFFER(Int, int);
DECLARE_BUFFER(String, ObjString*);

#endif // vessel_buffer_h