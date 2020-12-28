#pragma once

#include "object.h"
#include "compiler.h"
#include "scanner.h"
#include "memory.h"
#include "utils.h"
#include "vm.h"

#include <stdio.h>
#include <stdlib.h>

typedef struct
{
    ObjModule* module;

    Token current;
    Token previous;
    bool had_error;
    bool panic_mode;
} Parser;

typedef enum {
    PREC_NONE,
    PREC_ASSIGNMENT,  // =
    PREC_OR,          // or
    PREC_AND,         // and
    PREC_EQUALITY,    // == !=
    PREC_IS,          // is
    PREC_COMPARISON,  // < > <= >=
    PREC_TERM,        // + -
    PREC_FACTOR,      // * /
    PREC_UNARY,       // ! -
    PREC_CALL,        // . () []
    PREC_PRIMARY
} Precedence;

typedef void (*ParseFn)(bool can_assign);

typedef struct {
    ParseFn prefix;
    ParseFn infix;
    Precedence precedence;
} ParseRule;

typedef struct
{
    Token name;
    int depth;
    bool is_captured;
} Local;

typedef struct
{
    uint8_t index;
    bool is_local;
} Upvalue;

typedef enum
{
    TYPE_FUNCTION,
    TYPE_INITIALIZER,
    TYPE_METHOD,
    TYPE_SCRIPT
} FunctionType;

typedef struct Compiler
{
    struct Compiler* enclosing;
    ObjFunction* function;
    FunctionType type;

    Local locals[UINT8_COUNT];
    int local_count;
    Upvalue upvalues[UINT8_COUNT];
    int scope_depth;
} Compiler;

typedef struct ClassCompiler
{
    struct ClassCompiler* enclosing;
    Token name;
    bool has_superclass;
} ClassCompiler;

Parser parser;

Compiler* current = NULL;

ClassCompiler* current_class = NULL;

static Chunk* current_chunk()
{
    return &current->function->chunk;
}

static void error_at(Token* token, const char* message)
{
    if (parser.panic_mode) {
        return;
    }

    parser.panic_mode = true;

    fprintf(stderr, "[line %d] Error", token->line);

    if (token->type == TOKEN_EOF) {
        fprintf(stderr, " at end");
    } else if (token->type == TOKEN_ERROR) {
        // Nothing.
    } else {
        fprintf(stderr, " at '%.*s'", token->length, token->start);
    }

    fprintf(stderr, ": %s\n", message);
    parser.had_error = true;
}

static void error(const char* message)
{
    error_at(&parser.previous, message);
}

static void error_at_current(const char* message)
{
    error_at(&parser.current, message);
}

static void advance()
{
    parser.previous = parser.current;

    for (;;)
    {
        parser.current = scan_token();
        if (parser.current.type != TOKEN_ERROR) {
            break;
        }

        error_at_current(parser.current.start);
    }
}

static void consume(TokenType type, const char* message)
{
    if (parser.current.type == type) {
        advance();
        return;
    }

    error_at_current(message);
}

static bool check(TokenType type)
{
    return parser.current.type == type;
}

static bool match(TokenType type)
{
    if (!check(type)) {
        return false;
    }
    advance();
    return true;
}

static bool match_line()
{
    if (!match(TOKEN_LINE)) {
        return false;
    }

    while (match(TOKEN_LINE))
        ;
    return true;
}

static void ignore_new_lines()
{
    match_line();
}

static void emit_byte(uint8_t byte)
{
    write_chunk(current_chunk(), byte, parser.previous.line);
}

static void emit_bytes(uint8_t byte1, uint8_t byte2)
{
    emit_byte(byte1);
    emit_byte(byte2);
}

static void emit_short(int arg)
{
    emit_byte((arg >> 8) & 0xff);
    emit_byte(arg & 0xff);
}

static void emit_op(OpCode instruction)
{
    emit_byte(instruction);
}

static void emit_byte_arg(OpCode instruction, int arg)
{
    emit_op(instruction);
    emit_byte(arg);
}

static void emit_short_arg(OpCode instruction, int arg)
{
    emit_op(instruction);
    emit_short(arg);
}

static void emit_loop(int loop_start)
{
    emit_op(OP_LOOP);

    int offset = current_chunk()->count - loop_start + 2;
    if (offset > UINT16_MAX) {
        error("Loop body too large.");
    }

    emit_byte((offset >> 8) & 0xff);
    emit_byte(offset & 0xff);
}

static int emit_jump(uint8_t instruction)
{
    emit_byte(instruction);
    emit_byte(0xff);
    emit_byte(0xff);
    return current_chunk()->count - 2;
}

static void emit_return()
{
    if (current->type == TYPE_INITIALIZER) {
        emit_byte_arg(OP_GET_LOCAL, 0);
    } else {
        emit_op(OP_NIL);
    }

    emit_op(OP_RETURN);
}

static uint8_t make_constant(Value value)
{
    int constant = add_constant(current_chunk(), value);
    if (constant > UINT8_MAX) {
        error("Too many constants in one chunk.");
        return 0;
    }

    return (uint8_t)constant;
}

static void emit_constant(Value value)
{
    emit_byte_arg(OP_CONSTANT, make_constant(value));
}

static void patch_jump(int offset)
{
    // -2 to adjust for the bytecode for the jump offset itself.
    int jump = current_chunk()->count - offset - 2;

    if (jump > UINT16_MAX) {
        error("Too much code to jump over.");
    }

    current_chunk()->code[offset] = (jump >> 8) & 0xff;
    current_chunk()->code[offset + 1] = jump & 0xff;
}

static void init_compiler(Compiler* compiler, FunctionType type)
{
    compiler->enclosing = current;
    compiler->function = NULL;
    compiler->type = type;
    compiler->local_count = 0;
    compiler->scope_depth = 0;
    compiler->function = new_function(parser.module);
    current = compiler;

    if (type != TYPE_SCRIPT) {
        current->function->name = copy_string(parser.previous.start, parser.previous.length);
    }

    Local* local = &current->locals[current->local_count++];
    local->depth = 0;
    local->is_captured = false;
    if (type != TYPE_FUNCTION) {
        local->name.start = "this";
        local->name.length = 4;
    } else {
        local->name.start = "";
        local->name.length = 0;
    }
}

static ObjFunction* end_compiler()
{
    emit_return();
    ObjFunction* function = current->function;

#ifdef DEBUG_PRINT_CODE
    if (!parser.had_error) {
        disassembleChunk(current_chunk(), function->name != NULL ? function->name->chars : "<script>");
    }
#endif

    current = current->enclosing;
    return function;
}

static void begin_scope()
{
    current->scope_depth++;
}

static void end_scope()
{
    current->scope_depth--;

    while (current->local_count > 0
        && current->locals[current->local_count - 1].depth > current->scope_depth)
    {
        if (current->locals[current->local_count - 1].is_captured) {
            emit_op(OP_CLOSE_UPVALUE);
        } else {
            emit_op(OP_POP);
        }
        current->local_count--;
    }
}

static void expression();
static void statement();
static void declaration();

static ParseRule* get_rule(TokenType type);
static void parse_precedence(Precedence precedence);

static bool identifiers_equal(Token* a, Token* b)
{
    if (a->length != b->length) {
        return false;
    }
    return memcmp(a->start, b->start, a->length) == 0;
}

static uint8_t identifier_constant(Token* name)
{
    return make_constant(OBJ_VAL(copy_string(name->start, name->length)));
}

static int resolve_local(Compiler* compiler, Token* name)
{
    for (int i = compiler->local_count - 1; i >= 0; i--)
    {
        Local* local = &compiler->locals[i];
        if (identifiers_equal(name, &local->name)) {
            if (local->depth == -1) {
                error("Can't read local variable in its own initializer.");
            }
            return i;
        }
    }

    return -1;
}

static int add_upvalue(Compiler* compiler, uint8_t index, bool is_local)
{
    int upvalue_count = compiler->function->upvalue_count;

    for (int i = 0; i < upvalue_count; i++) {
        Upvalue* upvalue = &compiler->upvalues[i];
        if (upvalue->index == index && upvalue->is_local == is_local) {
            return i;
        }
    }

    if (upvalue_count == UINT8_COUNT) {
        error("Too many closure variables in function.");
        return 0;
    }

    compiler->upvalues[upvalue_count].is_local = is_local;
    compiler->upvalues[upvalue_count].index = index;
    return compiler->function->upvalue_count++;
}

static int resolve_upvalue(Compiler* compiler, Token* name)
{
    if (compiler->enclosing == NULL) {
        return -1;
    }

    int local = resolve_local(compiler->enclosing, name);
    if (local != -1) {
        compiler->enclosing->locals[local].is_captured = true;
        return add_upvalue(compiler, (uint8_t)local, true);
    }

    int upvalue = resolve_upvalue(compiler->enclosing, name);
    if (upvalue != -1) {
        return add_upvalue(compiler, (uint8_t)upvalue, false);
    }

    return -1;
}

static void add_local(Token name)
{
    if (current->local_count == UINT8_COUNT) {
        error("Too many local variables in function.");
        return;
    }

    Local* local = &current->locals[current->local_count++];
    local->name = name;
    local->depth = -1;
    local->is_captured = false;
}

static void declare_variable()
{
    if (current->scope_depth == 0) {
        return;
    }

    Token* name = &parser.previous;
    for (int i = current->local_count - 1; i >= 0; i--)
    {
        Local* local = &current->locals[i];
        if (local->depth != -1 && local->depth < current->scope_depth) {
            break; // [negative]
        }

        if (identifiers_equal(name, &local->name)) {
            error("Already variable with this name in this scope.");
        }
    }

    add_local(*name);
}

static uint8_t parse_variable(const char* errorMessage)
{
    consume(TOKEN_IDENTIFIER, errorMessage);

    declare_variable();
    if (current->scope_depth > 0) {
        return 0;
    }

    return identifier_constant(&parser.previous);
}

static void mark_initialized()
{
    if (current->scope_depth == 0) {
        return;
    }
    current->locals[current->local_count - 1].depth = current->scope_depth;
}

static void define_variable(uint8_t global)
{
    if (current->scope_depth > 0) {
        mark_initialized();
        return;
    }

    emit_byte_arg(OP_DEFINE_GLOBAL, global);
}

static uint8_t argument_list(TokenType token_right)
{
    uint8_t arg_count = 0;
    if (!check(token_right))
    {
        do {
            expression();

            if (arg_count == 255) {
                error("Can't have more than 255 arguments.");
            }
            arg_count++;
        } while (match(TOKEN_COMMA));
    }

    consume(token_right, "Expect ')' or ']' after arguments.");
    return arg_count;
}

static void and_(bool can_assign)
{
    ignore_new_lines();

    int end_jump = emit_jump(OP_JUMP_IF_FALSE);

    emit_op(OP_POP);
    parse_precedence(PREC_AND);

    patch_jump(end_jump);
}

static int signature_symbol(Signature* signature)
{
    char name[MAX_METHOD_SIGNATURE];
    int length;
    signature_to_string(signature, name, &length);

    return symbol_table_ensure(&vm.method_names, name, length);
}

static void call_signature(Signature* signature)
{
    int symbol = signature_symbol(signature);
    emit_short_arg((OpCode)(OP_CALL_0 + signature->arity), symbol);
}

static void binary(bool can_assign)
{
    TokenType operator_type = parser.previous.type;

    ParseRule* rule = get_rule(operator_type);
    parse_precedence((Precedence)(rule->precedence + 1));

    switch (operator_type)
    {
        case TOKEN_BANG_EQUAL:    emit_bytes(OP_EQUAL, OP_NOT); break;
        case TOKEN_EQUAL_EQUAL:   emit_op(OP_EQUAL); break;
        case TOKEN_GREATER:       emit_op(OP_GREATER); break;
        case TOKEN_GREATER_EQUAL: emit_bytes(OP_LESS, OP_NOT); break;
        case TOKEN_LESS:          emit_op(OP_LESS); break;
        case TOKEN_LESS_EQUAL:    emit_bytes(OP_GREATER, OP_NOT); break;
        case TOKEN_PLUS:          emit_op(OP_ADD); break;
        case TOKEN_MINUS:         emit_op(OP_SUBTRACT); break;
        case TOKEN_STAR:          emit_op(OP_MULTIPLY); break;
        case TOKEN_SLASH:         emit_op(OP_DIVIDE); break;
        case TOKEN_IS:
        {
            Signature signature = { "is", 2, SIG_METHOD, 1 };
            call_signature(&signature);
        }
            break;
        default:
            return; // Unreachable.
    }
}

static void call(bool can_assign)
{
    uint8_t arg_count = argument_list(TOKEN_RIGHT_PAREN);
    emit_byte_arg(OP_CALL, arg_count);
}

static void dot(bool can_assign)
{
    consume(TOKEN_IDENTIFIER, "Expect property name after '.'.");
    uint8_t name = identifier_constant(&parser.previous);

    if (can_assign && match(TOKEN_EQUAL)) {
        expression();
        emit_byte_arg(OP_SET_PROPERTY, name);
    } else if (match(TOKEN_LEFT_PAREN)) {
        uint8_t arg_count = argument_list(TOKEN_RIGHT_PAREN);
        emit_byte_arg(OP_INVOKE, name);
        emit_byte(arg_count);
    } else {
        emit_byte_arg(OP_GET_PROPERTY, name);
    }
}

static void literal(bool can_assign)
{
    switch (parser.previous.type)
    {
    case TOKEN_FALSE: emit_op(OP_FALSE); break;
    case TOKEN_NIL: emit_op(OP_NIL); break;
    case TOKEN_TRUE: emit_op(OP_TRUE); break;
    default:
        return; // Unreachable.
    }
}

static void grouping(bool can_assign)
{
    expression();
    consume(TOKEN_RIGHT_PAREN, "Expect ')' after expression.");
}

static void load_core_variable(const char* name)
{
    int symbol = symbol_table_find(&parser.module->variable_names, name, strlen(name));
    ASSERT(symbol != -1, "Should have already defined core name.");
    emit_short_arg(OP_LOAD_MODULE_VAR, symbol);
}

static void call_method(int num_args, const char* name, int length)
{
    int symbol = symbol_table_ensure(&vm.method_names, name, length);
    emit_short_arg((OpCode)(OP_CALL_0 + num_args), symbol);
}

static void list(bool can_assign)
{
    load_core_variable("List");
    call_method(0, "new()", 5);

    do
    {
        ignore_new_lines();

        if (check(TOKEN_RIGHT_BRACKET)) {
            break;
        }

        expression();
        call_method(1, "addCore_(_)", 11);
    } while (match(TOKEN_COMMA));

    ignore_new_lines();
    consume(TOKEN_RIGHT_BRACKET, "Expect ']' after list elements.");
}

static void subscript(bool can_assign)
{
    uint8_t arg_count = argument_list(TOKEN_RIGHT_BRACKET);
    Signature signature = { "", 0, SIG_SUBSCRIPT, arg_count };

    if (can_assign && match(TOKEN_EQUAL))
    {
        signature.type = SIG_SUBSCRIPT_SETTER;
        signature.arity += 1;
        expression();
    }

    call_signature(&signature);
}

static void map(bool canAssign)
{
    load_core_variable("Map");
    call_method(0, "new()", 5);

    do
    {
        ignore_new_lines();

        if (check(TOKEN_RIGHT_BRACE)) {
            break;
        }

        parse_precedence(PREC_UNARY);
        consume(TOKEN_COLON, "Expect ':' after map key.");
        ignore_new_lines();

        expression();
        call_method(2, "addCore_(_,_)", 13);
    } while (match(TOKEN_COMMA));

    ignore_new_lines();
    consume(TOKEN_RIGHT_BRACE, "Expect '}' after map entries.");
}

static void number(bool can_assign)
{
    double value = strtod(parser.previous.start, NULL);
    emit_constant(NUMBER_VAL(value));
}

static void or_(bool can_assign)
{
    ignore_new_lines();

    int else_jump = emit_jump(OP_JUMP_IF_FALSE);
    int end_jump = emit_jump(OP_JUMP);

    patch_jump(else_jump);
    emit_op(OP_POP);

    parse_precedence(PREC_OR);
    patch_jump(end_jump);
}

static void string(bool can_assign)
{
    emit_constant(OBJ_VAL(copy_string(parser.previous.start + 1, parser.previous.length - 2)));
}

static void named_variable(Token name, bool can_assign)
{
    uint8_t get_op, set_op;
    int arg = resolve_local(current, &name);
    if (arg != -1) {
        get_op = OP_GET_LOCAL;
        set_op = OP_SET_LOCAL;
    } else if ((arg = resolve_upvalue(current, &name)) != -1) {
        get_op = OP_GET_UPVALUE;
        set_op = OP_SET_UPVALUE;
    } else {
        arg = identifier_constant(&name);
        get_op = OP_GET_GLOBAL;
        set_op = OP_SET_GLOBAL;
    }

    if (can_assign && match(TOKEN_EQUAL)) {
        expression();
        emit_byte_arg(set_op, (uint8_t)arg);
    } else {
        emit_byte_arg(get_op, (uint8_t)arg);
    }
}

static void variable(bool can_assign)
{
    named_variable(parser.previous, can_assign);
}

static Token synthetic_token(const char* text)
{
    Token token;
    token.start = text;
    token.length = (int)strlen(text);
    return token;
}

static void super_(bool can_assign)
{
    if (current_class == NULL) {
        error("Can't use 'super' outside of a class.");
    } else if (!current_class->has_superclass) {
        error("Can't use 'super' in a class with no superclass.");
    }

    consume(TOKEN_DOT, "Expect '.' after 'super'.");
    consume(TOKEN_IDENTIFIER, "Expect superclass method name.");
    uint8_t name = identifier_constant(&parser.previous);

    named_variable(synthetic_token("this"), false);
    if (match(TOKEN_LEFT_PAREN)) {
        uint8_t arg_count = argument_list(TOKEN_RIGHT_PAREN);
        named_variable(synthetic_token("super"), false);
        emit_byte_arg(OP_SUPER_INVOKE, name);
        emit_byte(arg_count);
    } else {
        named_variable(synthetic_token("super"), false);
        emit_byte_arg(OP_GET_SUPER, name);
    }
}

static void this_(bool can_assign)
{
    if (current_class == NULL) {
        error("Can't use 'this' outside of a class.");
        return;
    }
    variable(false);
}

static void unary(bool can_assign)
{
    TokenType operator_type = parser.previous.type;

    parse_precedence(PREC_UNARY);

    switch (operator_type)
    {
        case TOKEN_BANG: emit_op(OP_NOT); break;
        case TOKEN_MINUS: emit_op(OP_NEGATE); break;
        default:
            return; // Unreachable.
    }
}

ParseRule rules[] =
{
    [TOKEN_LEFT_PAREN]    = {grouping, call,   PREC_CALL},
    [TOKEN_RIGHT_PAREN]   = {NULL,     NULL,   PREC_NONE},
    [TOKEN_LEFT_BRACKET]  = {list,     subscript, PREC_CALL},
    [TOKEN_RIGHT_BRACKET] = {NULL,     NULL,   PREC_NONE},
    [TOKEN_LEFT_BRACE]    = {map,      NULL,   PREC_NONE},
    [TOKEN_RIGHT_BRACE]   = {NULL,     NULL,   PREC_NONE},
    [TOKEN_COLON]         = {NULL,     NULL,   PREC_NONE},
    [TOKEN_COMMA]         = {NULL,     NULL,   PREC_NONE},
    [TOKEN_DOT]           = {NULL,     dot,    PREC_CALL},
    [TOKEN_MINUS]         = {unary,    binary, PREC_TERM},
    [TOKEN_PLUS]          = {NULL,     binary, PREC_TERM},
    [TOKEN_SEMICOLON]     = {NULL,     NULL,   PREC_NONE},
    [TOKEN_SLASH]         = {NULL,     binary, PREC_FACTOR},
    [TOKEN_STAR]          = {NULL,     binary, PREC_FACTOR},
    [TOKEN_BANG]          = {unary,    NULL,   PREC_NONE},
    [TOKEN_BANG_EQUAL]    = {NULL,     binary, PREC_EQUALITY},
    [TOKEN_EQUAL]         = {NULL,     NULL,   PREC_NONE},
    [TOKEN_EQUAL_EQUAL]   = {NULL,     binary, PREC_EQUALITY},
    [TOKEN_GREATER]       = {NULL,     binary, PREC_COMPARISON},
    [TOKEN_GREATER_EQUAL] = {NULL,     binary, PREC_COMPARISON},
    [TOKEN_LESS]          = {NULL,     binary, PREC_COMPARISON},
    [TOKEN_LESS_EQUAL]    = {NULL,     binary, PREC_COMPARISON},
    [TOKEN_IDENTIFIER]    = {variable, NULL,   PREC_NONE},
    [TOKEN_STRING]        = {string,   NULL,   PREC_NONE},
    [TOKEN_NUMBER]        = {number,   NULL,   PREC_NONE},
    [TOKEN_AND]           = {NULL,     and_,   PREC_AND},
    [TOKEN_CLASS]         = {NULL,     NULL,   PREC_NONE},
    [TOKEN_ELSE]          = {NULL,     NULL,   PREC_NONE},
    [TOKEN_FALSE]         = {literal,  NULL,   PREC_NONE},
    [TOKEN_FOR]           = {NULL,     NULL,   PREC_NONE},
    [TOKEN_FUN]           = {NULL,     NULL,   PREC_NONE},
    [TOKEN_IF]            = {NULL,     NULL,   PREC_NONE},
    [TOKEN_NIL]           = {literal,  NULL,   PREC_NONE},
    [TOKEN_OR]            = {NULL,     or_,    PREC_OR},
    [TOKEN_PRINT]         = {NULL,     NULL,   PREC_NONE},
    [TOKEN_RETURN]        = {NULL,     NULL,   PREC_NONE},
    [TOKEN_SUPER]         = {super_,   NULL,   PREC_NONE},
    [TOKEN_THIS]          = {this_,    NULL,   PREC_NONE},
    [TOKEN_TRUE]          = {literal,  NULL,   PREC_NONE},
    [TOKEN_VAR]           = {NULL,     NULL,   PREC_NONE},
    [TOKEN_WHILE]         = {NULL,     NULL,   PREC_NONE},
    [TOKEN_IMPORT]        = {NULL,     NULL,   PREC_NONE},
    [TOKEN_IS]            = {NULL,     binary, PREC_IS},
    [TOKEN_ERROR]         = {NULL,     NULL,   PREC_NONE},
    [TOKEN_EOF]           = {NULL,     NULL,   PREC_NONE},
};

static void parse_precedence(Precedence precedence)
{
    advance();
    ParseFn prefix_rule = get_rule(parser.previous.type)->prefix;
    if (prefix_rule == NULL) {
        error("Expect expression.");
        return;
    }

    bool can_assign = precedence <= PREC_ASSIGNMENT;
    prefix_rule(can_assign);

    while (precedence <= get_rule(parser.current.type)->precedence) {
        advance();
        ParseFn infix_rule = get_rule(parser.previous.type)->infix;
        infix_rule(can_assign);
    }

    if (can_assign && match(TOKEN_EQUAL)) {
        error("Invalid assignment target.");
    }
}

static ParseRule* get_rule(TokenType type)
{
    return &rules[type];
}

static void expression()
{
    parse_precedence(PREC_ASSIGNMENT);
}

static void block()
{
    while (!check(TOKEN_RIGHT_BRACE) && !check(TOKEN_EOF)) {
        declaration();
    }

    consume(TOKEN_RIGHT_BRACE, "Expect '}' after block.");
}

static void function(FunctionType type)
{
    Compiler compiler;
    init_compiler(&compiler, type);
    begin_scope(); // [no-end-scope]

    consume(TOKEN_LEFT_PAREN, "Expect '(' after function name.");
    if (!check(TOKEN_RIGHT_PAREN))
    {
        do {
            current->function->arity++;
            if (current->function->arity > 255) {
                error_at_current("Can't have more than 255 parameters.");
            }

            uint8_t paramConstant = parse_variable("Expect parameter name.");
            define_variable(paramConstant);
        } while (match(TOKEN_COMMA));
    }
    consume(TOKEN_RIGHT_PAREN, "Expect ')' after parameters.");

    consume(TOKEN_LEFT_BRACE, "Expect '{' before function body.");
    block();

    ObjFunction* function = end_compiler();
    emit_byte_arg(OP_CLOSURE, make_constant(OBJ_VAL(function)));

    for (int i = 0; i < function->upvalue_count; i++) {
        emit_byte(compiler.upvalues[i].is_local ? 1 : 0);
        emit_byte(compiler.upvalues[i].index);
    }
}

static void method()
{
    consume(TOKEN_IDENTIFIER, "Expect method name.");
    uint8_t constant = identifier_constant(&parser.previous);

    FunctionType type = TYPE_METHOD;
    if (parser.previous.length == 4 &&
        memcmp(parser.previous.start, "init", 4) == 0) {
        type = TYPE_INITIALIZER;
    }

    function(type);
    emit_byte_arg(OP_METHOD, constant);
}

static void class_declaration()
{
    consume(TOKEN_IDENTIFIER, "Expect class name.");
    Token class_name = parser.previous;
    uint8_t name_constant = identifier_constant(&parser.previous);
    declare_variable();

    emit_byte_arg(OP_CLASS, name_constant);
    define_variable(name_constant);

    ClassCompiler class_compiler;
    class_compiler.name = parser.previous;
    class_compiler.has_superclass = false;
    class_compiler.enclosing = current_class;
    current_class = &class_compiler;

    if (match(TOKEN_LESS))
    {
        consume(TOKEN_IDENTIFIER, "Expect superclass name.");
        variable(false);

        if (identifiers_equal(&class_name, &parser.previous)) {
            error("A class can't inherit from itself.");
        }

        begin_scope();
        add_local(synthetic_token("super"));
        define_variable(0);

        named_variable(class_name, false);
        emit_op(OP_INHERIT);
        class_compiler.has_superclass = true;
    }

    named_variable(class_name, false);
    consume(TOKEN_LEFT_BRACE, "Expect '{' before class body.");
    while (!check(TOKEN_RIGHT_BRACE) && !check(TOKEN_EOF)) {
        ignore_new_lines();
        method();
        ignore_new_lines();
    }
    consume(TOKEN_RIGHT_BRACE, "Expect '}' after class body.");
    emit_op(OP_POP);

    if (class_compiler.has_superclass) {
        end_scope();
    }

    current_class = current_class->enclosing;
}

static void fun_declaration()
{
    uint8_t global = parse_variable("Expect function name.");
    mark_initialized();
    function(TYPE_FUNCTION);
    define_variable(global);
}

static void var_declaration()
{
    uint8_t global = parse_variable("Expect variable name.");

    if (match(TOKEN_EQUAL)) {
        expression();
    } else {
        emit_op(OP_NIL);
    }

    define_variable(global);
}

static void expression_statement()
{
    expression();
    emit_op(OP_POP);
}

static void for_statement()
{
    begin_scope();

    consume(TOKEN_LEFT_PAREN, "Expect '(' after 'for'.");
    // for-initializer
    if (match(TOKEN_SEMICOLON)) {
        // No initializer.
    } else if (match(TOKEN_VAR)) {
        var_declaration();
        consume(TOKEN_SEMICOLON, "Expect ';' after loop init.");
    } else {
        expression_statement();
        consume(TOKEN_SEMICOLON, "Expect ';' after loop init.");
    }

    int loop_start = current_chunk()->count;

    // for-exit
    int exit_jump = -1;
    if (!match(TOKEN_SEMICOLON))
    {
        expression();
        consume(TOKEN_SEMICOLON, "Expect ';' after loop condition.");

        exit_jump = emit_jump(OP_JUMP_IF_FALSE);
        emit_op(OP_POP); // Condition.
    }

    // for-increment
    if (!match(TOKEN_RIGHT_PAREN))
    {
        int body_jump = emit_jump(OP_JUMP);

        int increment_start = current_chunk()->count;
        expression();
        emit_op(OP_POP);
        consume(TOKEN_RIGHT_PAREN, "Expect ')' after for clauses.");

        emit_loop(loop_start);
        loop_start = increment_start;
        patch_jump(body_jump);
    }

    statement();

    emit_loop(loop_start);

    if (exit_jump != -1)
    {
        patch_jump(exit_jump);
        emit_op(OP_POP); // Condition.
    }

    end_scope();
}

static void if_statement()
{
    consume(TOKEN_LEFT_PAREN, "Expect '(' after 'if'.");
    expression();
    consume(TOKEN_RIGHT_PAREN, "Expect ')' after condition."); // [paren]

    int then_jump = emit_jump(OP_JUMP_IF_FALSE);
    emit_op(OP_POP);
    statement();

    int else_jump = emit_jump(OP_JUMP);

    patch_jump(then_jump);
    emit_op(OP_POP);

    if (match(TOKEN_ELSE)) {
        statement();
    }
    patch_jump(else_jump);
}

static void print_statement()
{
    expression();
    emit_op(OP_PRINT);
}

static void return_statement()
{
    if (current->type == TYPE_SCRIPT) {
        error("Can't return from top-level code.");
    }

    if (match(TOKEN_LINE)) {
        emit_return();
    } else {
        if (current->type == TYPE_INITIALIZER) {
            error("Can't return a value from an initializer.");
        }

        expression();
        emit_op(OP_RETURN);
    }
}

static void while_statement()
{
    int loop_start = current_chunk()->count;

    consume(TOKEN_LEFT_PAREN, "Expect '(' after 'while'.");
    expression();
    consume(TOKEN_RIGHT_PAREN, "Expect ')' after condition.");

    int exit_jump = emit_jump(OP_JUMP_IF_FALSE);

    emit_op(OP_POP);
    statement();

    emit_loop(loop_start);

    patch_jump(exit_jump);
    emit_op(OP_POP);
}

static void synchronize()
{
    parser.panic_mode = false;

    while (parser.current.type != TOKEN_EOF)
    {
        if (parser.previous.type == TOKEN_LINE) {
            return;
        }

        switch (parser.current.type)
        {
            case TOKEN_CLASS:
            case TOKEN_FUN:
            case TOKEN_VAR:
            case TOKEN_FOR:
            case TOKEN_IF:
            case TOKEN_WHILE:
            case TOKEN_PRINT:
            case TOKEN_RETURN:
            return;

            default:
            // Do nothing.
            ;
        }

        advance();
    }
}

static void import()
{
    ignore_new_lines();
    consume(TOKEN_STRING, "Expect a string after 'import'.");

    int module_constant = make_constant(OBJ_VAL(copy_string(parser.previous.start + 1, parser.previous.length - 2)));

    // Load the module.
    emit_byte_arg(OP_IMPORT_MODULE, module_constant);

    // Discard the unused result value from calling the module body's closure.
    emit_op(OP_POP);

    // The for clause is optional.
    if (!match(TOKEN_FOR)) {
        return;
    }

    // Compile the comma-separated list of variables to import.
    do
    {
        ignore_new_lines();

        consume(TOKEN_IDENTIFIER, "Expect variable name.");

        // We need to hold onto the source variable,
        // in order to reference it in the import later
        Token sourceVariableToken = parser.previous;

        // Define a string constant for the original variable name.
        int sourceVariableConstant = make_constant(OBJ_VAL(copy_string(sourceVariableToken.start, sourceVariableToken.length)));

        // Store the symbol we care about for the variable
        int slot = -1;
        //if(match(TOKEN_AS))
        //{
        //    //import "module" for Source as Dest
        //    //Use 'Dest' as the name by declaring a new variable for it.
        //    //This parses a name after the 'as' and defines it.
        //    slot = declareNamedVariable(compiler);
        //}
        //else
        {
            //import "module" for Source
            //Uses 'Source' as the name directly
            declare_variable(&sourceVariableToken);
        }

        // Load the variable from the other module.
        emit_byte_arg(OP_IMPORT_VARIABLE, sourceVariableConstant);

        // Store the result in the variable here.
        define_variable(sourceVariableConstant);
    } while (match(TOKEN_COMMA));
}

static void declaration()
{
    ignore_new_lines();
    if (match(TOKEN_EOF)) {
        return;
    }

    if (match(TOKEN_CLASS)) {
        class_declaration();
    } else if (match(TOKEN_FUN)) {
        fun_declaration();
    } else if (match(TOKEN_VAR)) {
        var_declaration();
    } else if (match(TOKEN_IMPORT)) {
        import();
    } else {
        statement();
    }

    if (parser.panic_mode) {
        synchronize();
    }

    ignore_new_lines();
}

static void statement()
{
    ignore_new_lines();

    if (match(TOKEN_PRINT)) {
        print_statement();
    } else if (match(TOKEN_FOR)) {
        for_statement();
    } else if (match(TOKEN_IF)) {
        if_statement();
    } else if (match(TOKEN_RETURN)) {
        return_statement();
    } else if (match(TOKEN_WHILE)) {
        while_statement();
    } else if (match(TOKEN_LEFT_BRACE)) {
        begin_scope();
        block();
        end_scope();
    } else {
        expression_statement();
    }

    ignore_new_lines();
}

static ObjFunction* compile_impl(ObjModule* module, const char* source)
{
    init_scanner(source);

    Compiler compiler;
    init_compiler(&compiler, TYPE_SCRIPT);

    parser.module = module;
    parser.had_error = false;
    parser.panic_mode = false;

    advance();

    while (!match(TOKEN_EOF)) {
        declaration();
    }

    ObjFunction* function = end_compiler();
    return parser.had_error ? NULL : function;
}

static ObjModule* get_module(Value name)
{
    Value module;
    if (table_get(&vm.modules, AS_STRING(name), &module)) {
        return AS_MODULE(module);
    } else {
        return NULL;
    }
}

ObjFunction* compile(const char* module, const char* source)
{
    ObjModule* obj_module = NULL;

    ObjString* module_str = copy_string(module, strlen(module));

    Value v_module;
    if (table_get(&vm.modules, module_str, &v_module)) {
        obj_module = AS_MODULE(v_module);
    }

    if (obj_module == NULL)
    {
        obj_module = new_module(module_str);

        push(OBJ_VAL(obj_module));
        table_set(&vm.modules, module_str, OBJ_VAL(obj_module));
        pop();

        Value v_core_module;
        if (!table_get(&vm.modules, copy_string("Core", 4), &v_core_module)) {
            error("Core module should be loaded.");
        }

        ObjModule* core_module = AS_MODULE(v_core_module);
        for (int i = 0; i < core_module->variables.count; ++i)
        {
            ObjString* name = AS_STRING(core_module->variable_names.values[i]);
            DefineVariable(obj_module, name->chars, name->length, core_module->variables.values[i], NULL);
        }
    }

    if (obj_module != NULL) {
        push(v_module);
    }
    ObjFunction* func = compile_impl(obj_module, source);
    if (obj_module != NULL) {
        pop();
    }

    return func;
}

void mark_compiler_roots() {
    Compiler* compiler = current;
    while (compiler != NULL) {
        mark_object((Obj*)compiler->function);
        compiler = compiler->enclosing;
    }
}

static void signature_parameter_list(char name[MAX_METHOD_SIGNATURE], int* length,
                                     int num_params, char left_bracket, char right_bracket)
{
	name[(*length)++] = left_bracket;

	for (int i = 0; i < num_params && i < MAX_PARAMETERS; i++)
	{
		if (i > 0) {
			name[(*length)++] = ',';
		}
		name[(*length)++] = '_';
	}
	name[(*length)++] = right_bracket;
}

void signature_to_string(Signature* signature, char name[MAX_METHOD_SIGNATURE], int* length)
{
    *length = 0;

    memcpy(name + *length, signature->name, signature->length);
    *length += signature->length;

    switch (signature->type)
    {
    case SIG_METHOD:
        signature_parameter_list(name, length, signature->arity, '(', ')');
        break;

    case SIG_GETTER:
        break;

    case SIG_SETTER:
        name[(*length)++] = '=';
        signature_parameter_list(name, length, 1, '(', ')');
        break;

    case SIG_SUBSCRIPT:
        signature_parameter_list(name, length, signature->arity, '[', ']');
        break;

    case SIG_SUBSCRIPT_SETTER:
        signature_parameter_list(name, length, signature->arity - 1, '[', ']');
        name[(*length)++] = '=';
        signature_parameter_list(name, length, 1, '(', ')');
        break;

    case SIG_INITIALIZER:
        memcpy(name, "init ", 5);
        memcpy(name + 5, signature->name, signature->length);
        *length = 5 + signature->length;
        signature_parameter_list(name, length, signature->arity, '(', ')');
        break;
    }

    name[*length] = '\0';
}