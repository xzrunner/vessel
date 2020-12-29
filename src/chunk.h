#ifndef vessel_chunk_h
#define vessel_chunk_h

#include "common.h"
#include "value.h"

#include <stdint.h>

typedef struct
{
	int count;
	int capacity;
	uint8_t* code;
	int* lines;
	ValueArray constants;
} Chunk;

void init_chunk(Chunk* chunk);
void free_chunk(Chunk* chunk);
void write_chunk(Chunk* chunk, uint8_t byte, int line);
int add_constant(Chunk* chunk, Value value);

#endif // vessel_chunk_h
