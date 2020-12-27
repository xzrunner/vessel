#ifndef vessel_chunk_h
#define vessel_chunk_h

#include "common.h"
#include "value.h"

#include <stdint.h>

typedef enum
{
	OP_CONSTANT,
	OP_NIL,
	OP_TRUE,
	OP_FALSE,
	OP_POP,
	OP_GET_LOCAL,
	OP_SET_LOCAL,
	OP_GET_GLOBAL,
	OP_DEFINE_GLOBAL,
	OP_SET_GLOBAL,
	OP_GET_UPVALUE,
	OP_SET_UPVALUE,
	OP_GET_PROPERTY,
	OP_SET_PROPERTY,
	OP_GET_SUPER,
	OP_EQUAL,
	OP_GREATER,
	OP_LESS,
	OP_ADD,
	OP_SUBTRACT,
	OP_MULTIPLY,
	OP_DIVIDE,
	OP_NOT,
	OP_NEGATE,
	OP_PRINT,
	OP_JUMP,
	OP_JUMP_IF_FALSE,
	OP_LOOP,
	OP_CALL,
	OP_INVOKE,
	OP_SUPER_INVOKE,
	OP_CLOSURE,
	OP_CLOSE_UPVALUE,
	OP_RETURN,
	OP_CLASS,
	OP_INHERIT,
	OP_METHOD,
	OP_LOAD_MODULE_VAR,
	OP_CALL_0,
	OP_CALL_1,
	OP_CALL_2,
	OP_CALL_3,
	OP_CALL_4,
	OP_CALL_5,
	OP_CALL_6,
	OP_CALL_7,
	OP_CALL_8,
	OP_CALL_9,
	OP_CALL_10,
	OP_CALL_11,
	OP_CALL_12,
	OP_CALL_13,
	OP_CALL_14,
	OP_CALL_15,
	OP_CALL_16,
	OP_IMPORT_MODULE,
	OP_IMPORT_VARIABLE,
} OpCode;

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
