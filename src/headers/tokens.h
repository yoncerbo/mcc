#ifndef INCLUDE_TOKENS
#define INCLUDE_TOKENS

#include <stdint.h>

#define MAX_TOKENS 256

typedef enum {
  TOK_NONE,

  TOK_IDENT,
  TOK_DECIMAL,

  TOK_INT,
  TOK_VOID,
  TOK_RETURN,

  TOK_LPAREN,
  TOK_RPAREN,
  TOK_LBRACE,
  TOK_RBRACE,
  TOK_SEMICOLON,
  TOK_MINUS,
  TOK_PLUS,

  TOK_COUNT,
} TokenType;

const char *TOKEN_TYPE_STR[TOK_COUNT] = {
  "TOK_NONE",
  "TOK_IDENT",
  "TOK_DECIMAL",
  "TOK_INT",
  "TOK_VOID",
  "TOK_RETURN",
  "TOK_LPAREN",
  "TOK_RPAREN",
  "TOK_LBRACE",
  "TOK_RBRACE",
  "TOK_SEMICOLON",
  "TOK_MINUS",
  "TOK_PLUS",
};

typedef struct {
  TokenType type;
  uint16_t len;
  uint32_t start;
} Token;

void tokenize(const char *source, Token tokens_out[MAX_TOKENS]);
void print_tokens(Token *tokens);

#endif

