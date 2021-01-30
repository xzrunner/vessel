#include "scanner.h"
#include "object.h"
#include "buffer.h"

#include <stdbool.h>
#include <string.h>

#define MAX_INTERPOLATION_NESTING 8

typedef struct
{
	const char* start;
	const char* current;
	int line;

    int parens[MAX_INTERPOLATION_NESTING];
    int num_parens;

} Scanner;

Scanner scanner;

void init_scanner(const char* source)
{
	scanner.start = source;
	scanner.current = source;
	scanner.line = 1;
    scanner.num_parens = 0;
}

static bool is_alpha(char c)
{
    return (c >= 'a' && c <= 'z') ||
           (c >= 'A' && c <= 'Z') ||
            c == '_';
}

static bool is_digit(char c)
{
    return c >= '0' && c <= '9';
}

static bool is_at_end()
{
    return *scanner.current == '\0';
}

static char peek()
{
    return *scanner.current;
}

static char peek_next()
{
    if (is_at_end()) {
        return '\0';
    }
    return scanner.current[1];
}

static char advance()
{
    char c = peek();
    scanner.current++;
    if (c == '\n') {
        scanner.line++;
    }
    return c;
}

static bool match(char expected)
{
    if (is_at_end()) {
        return false;
    }
    if (*scanner.current != expected) {
        return false;
    }

    scanner.current++;
    return true;
}

static Token make_token(TokenType type)
{
    Token token;
    token.type = type;
    token.start = scanner.start;
    token.length = (int)(scanner.current - scanner.start);
    token.line = scanner.line;

    return token;
}

static Token error_token(const char* message)
{
    Token token;
    token.type = TOKEN_ERROR;
    token.start = message;
    token.length = (int)strlen(message);
    token.line = scanner.line;

    return token;
}

static void skip_whitespace()
{
    for (;;)
    {
        char c = peek();
        switch (c) {
        case ' ':
        case '\r':
        case '\t':
            advance();
            break;

        case '/':
            if (peek_next() == '/') {
                while (peek() != '\n' && !is_at_end()) {
                    advance();
                }
            } else {
                return;
            }
            break;

        default:
            return;
        }
    }
}

static TokenType check_keyword(int start, int length, const char* rest, TokenType type)
{
    if (scanner.current - scanner.start == start + length &&
        memcmp(scanner.start + start, rest, length) == 0) {
        return type;
    }

    return TOKEN_IDENTIFIER;
}

static TokenType identifier_type()
{
    switch (scanner.start[0])
    {
        case 'a':
            if (scanner.current - scanner.start > 1)
            {
                switch (scanner.start[1])
                {
                case 'n': return check_keyword(2, 1, "d", TOKEN_FALSE);
                case 's': return check_keyword(2, 0, "", TOKEN_AS);
                }
            }
            break;
        case 'c': return check_keyword(1, 4, "lass", TOKEN_CLASS);
        case 'e': return check_keyword(1, 3, "lse", TOKEN_ELSE);
        case 'f':
            if (scanner.current - scanner.start > 1)
            {
                switch (scanner.start[1])
                {
                case 'a': return check_keyword(2, 3, "lse", TOKEN_FALSE);
                case 'u': return check_keyword(2, 1, "n", TOKEN_FUN);
                case 'o':
                {
                    if (scanner.current - scanner.start > 3) {
                        return check_keyword(2, 5, "reign", TOKEN_FOREIGN);
                    } else {
                        return check_keyword(2, 1, "r", TOKEN_FOR);
                    }
                }
                    break;
                }
            }
            break;
        case 'i':
            if (scanner.current - scanner.start > 1)
            {
                switch (scanner.start[1])
                {
                case 'f': return check_keyword(2, 0, "", TOKEN_IF);
                case 'm': return check_keyword(2, 4, "port", TOKEN_IMPORT);
                case 'n': return check_keyword(2, 0, "", TOKEN_IN);
                case 's': return check_keyword(2, 0, "", TOKEN_IS);
                }
            }
            break;
        case 'n': return check_keyword(1, 2, "il", TOKEN_NIL);
        case 'o': return check_keyword(1, 1, "r", TOKEN_OR);
        case 'r': return check_keyword(1, 5, "eturn", TOKEN_RETURN);
        case 's':
            if (scanner.current - scanner.start > 1)
            {
                switch (scanner.start[1])
                {
                case 't': return check_keyword(2, 4, "atic", TOKEN_STATIC);
                case 'u': return check_keyword(2, 3, "per", TOKEN_SUPER);
                }
            }
        case 't':
            if (scanner.current - scanner.start > 1)
            {
                switch (scanner.start[1])
                {
                case 'h': return check_keyword(2, 2, "is", TOKEN_THIS);
                case 'r': return check_keyword(2, 2, "ue", TOKEN_TRUE);
                }
            }
            break;
        case 'v': return check_keyword(1, 2, "ar", TOKEN_VAR);
        case 'w': return check_keyword(1, 4, "hile", TOKEN_WHILE);
    }

    return TOKEN_IDENTIFIER;
}

static Token identifier()
{
    while (is_alpha(peek()) || is_digit(peek())) {
        advance();
    }

    return make_token(identifier_type());
}

static Token number()
{
    while (is_digit(peek())) {
        advance();
    }

    if (peek() == '.' && is_digit(peek_next())) {
        advance();  // Consume the ".".

        while (is_digit(peek())) {
            advance();
        }
    }

    return make_token(TOKEN_NUMBER);
}

static Token string()
{
    TokenType type = TOKEN_STRING;

    ByteBuffer string;
    ByteBufferInit(&string);

    ByteBufferWrite(&string, '"');
    for (;;)
    {
        char c = advance();
        if (c == '"') {
            break;
        }

        if (is_at_end()) {
            return error_token("Unterminated string.");
        }

        if (c == '%')
        {
            if (scanner.num_parens < MAX_INTERPOLATION_NESTING)
            {
                if (advance() != '(') {
                    error_token("Expect '(' after '%%'.");
                }

                scanner.parens[scanner.num_parens++] = 1;
                type = TOKEN_INTERPOLATION;
                break;
            }

            error_token("Interpolation too many levels.");
        }

        if (c == '\\')
        {
            switch (advance())
            {
                case '"':  ByteBufferWrite(&string, '"'); break;
                case '\\': ByteBufferWrite(&string, '\\'); break;
                case '%':  ByteBufferWrite(&string, '%'); break;
                case '0':  ByteBufferWrite(&string, '\0'); break;
                case 'a':  ByteBufferWrite(&string, '\a'); break;
                case 'b':  ByteBufferWrite(&string, '\b'); break;
                case 'f':  ByteBufferWrite(&string, '\f'); break;
                case 'n':  ByteBufferWrite(&string, '\n'); break;
                case 'r':  ByteBufferWrite(&string, '\r'); break;
                case 't':  ByteBufferWrite(&string, '\t'); break;
                case 'v':  ByteBufferWrite(&string, '\v'); break;

                default:
                    error_token("Invalid escape character.");
                    break;
            }
        }
        else
        {
            ByteBufferWrite(&string, c);
        }
    }
    ByteBufferWrite(&string, '"');

    ObjString* obj_str = copy_string(string.data, string.count);

    Token token;
    token.type = type;
    token.start = obj_str->chars;
    token.length = obj_str->length;
    token.line = scanner.line;

    return token;
}

Token scan_token()
{
    skip_whitespace();

    scanner.start = scanner.current;

    if (is_at_end()) {
        return make_token(TOKEN_EOF);
    }

    char c = advance();
    if (is_alpha(c)) {
        return identifier();
    }
    if (is_digit(c)) {
        return number();
    }

    switch (c)
    {
    case '(':
        if (scanner.num_parens > 0) {
            scanner.parens[scanner.num_parens - 1]++;
        }
        return make_token(TOKEN_LEFT_PAREN);
    case ')':
        if (scanner.num_parens > 0 && --scanner.parens[scanner.num_parens - 1] == 0) {
            scanner.num_parens--;
            return string();
        }
        return make_token(TOKEN_RIGHT_PAREN);
    case '[': return make_token(TOKEN_LEFT_BRACKET);
    case ']': return make_token(TOKEN_RIGHT_BRACKET);
    case '{': return make_token(TOKEN_LEFT_BRACE);
    case '}': return make_token(TOKEN_RIGHT_BRACE);
    case ':': return make_token(TOKEN_COLON);
    case ';': return make_token(TOKEN_SEMICOLON);
    case ',': return make_token(TOKEN_COMMA);
    case '.':
    {
        if (match('.')) {
            return make_token(match('=') ? TOKEN_DOTDOT_EQUAL : TOKEN_DOTDOT);
        } else {
            return make_token(TOKEN_DOT);
        }
    }
    case '-': return make_token(TOKEN_MINUS);
    case '+': return make_token(TOKEN_PLUS);
    case '/': return make_token(TOKEN_SLASH);
    case '*': return make_token(TOKEN_STAR);

    case '\n': return make_token(TOKEN_LINE);

    case '!': return make_token(match('=') ? TOKEN_BANG_EQUAL : TOKEN_BANG);
    case '=': return make_token(match('=') ? TOKEN_EQUAL_EQUAL : TOKEN_EQUAL);
    case '<': return make_token(match('=') ? TOKEN_LESS_EQUAL : TOKEN_LESS);
    case '>': return make_token(match('=') ? TOKEN_GREATER_EQUAL : TOKEN_GREATER);

    case '"': return string();
    }

    return error_token("Unexpected character.");
}