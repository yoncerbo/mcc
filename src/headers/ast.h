#ifndef INCLUDE_AST
#define INCLUDE_AST

#include <stdint.h>
#include "common.h"
#include "tokens.h"

#define MAX_AST_SIZE 512

typedef enum {
  AST_NONE,

  // Binary operations
  AST_MUL,
  AST_DIV,
  AST_MOD,
  AST_ADD,
  AST_SUB,
  AST_LSFT,
  AST_RSFT,
  AST_LT,
  AST_LE,
  AST_GT,
  AST_GE,
  AST_EQ,
  AST_NE,
  AST_BAND,
  AST_BXOR,
  AST_BOR,
  AST_LAND,
  AST_LOR,

  // Assignment operators
  AST_ASS,
  AST_ASS_MUL,
  AST_ASS_DIV,
  AST_ASS_MOD,
  AST_ASS_ADD,
  AST_ASS_SUB,
  AST_ASS_LSFT,
  AST_ASS_RSFT,
  AST_ASS_AND,
  AST_ASS_XOR,
  AST_ASS_OR,

  // Prefix expressions
  AST_INDEX,
  AST_CALL,
  AST_DOT,
  AST_ARROW,
  AST_POST_INC,
  AST_POST_DEC,

  // Postfix expressions
  AST_PRE_INC,
  AST_PRE_DEC,
  AST_ADDR,
  AST_DEREF,
  AST_PLUS,
  AST_MINUS,
  AST_NEG,
  AST_NOT,
  AST_SIZEOF,

  // Other expressions
  AST_IDENT,
  AST_INT,
  AST_CONDITIONAL,

  // Statements
  AST_LABEL,
  AST_CASE,
  AST_DEFAULT,
  AST_COMPOUND,
  AST_EMPTY,
  AST_IF,
  AST_SWITCH,
  AST_WHILE,
  AST_DO_WHILE,
  AST_FOR,
  AST_GOTO,
  AST_CONTINUE,
  AST_BREAK,
  AST_RETURN,

  AST_COUNT,
} AstType;

const char *AST_TYPE_STR[AST_COUNT] = {
  "AST_NONE",
  "AST_MUL",
  "AST_DIV",
  "AST_MOD",
  "AST_ADD",
  "AST_SUB",
  "AST_LSFT",
  "AST_RSFT",
  "AST_LT",
  "AST_LE",
  "AST_GT",
  "AST_GE",
  "AST_EQ",
  "AST_NE",
  "AST_BAND",
  "AST_BXOR",
  "AST_BOR",
  "AST_LAND",
  "AST_LOR",
  "AST_ASS",
  "AST_ASS_MUL",
  "AST_ASS_DIV",
  "AST_ASS_MOD",
  "AST_ASS_ADD",
  "AST_ASS_SUB",
  "AST_ASS_LSFT",
  "AST_ASS_RSFT",
  "AST_ASS_AND",
  "AST_ASS_XOR",
  "AST_ASS_OR",
  "AST_INDEX",
  "AST_CALL",
  "AST_DOT",
  "AST_ARROW",
  "AST_POST_INC",
  "AST_POST_DEC",
  "AST_PRE_INC",
  "AST_PRE_DEC",
  "AST_ADDR",
  "AST_DEREF",
  "AST_PLUS",
  "AST_MINUS",
  "AST_NEG",
  "AST_NOT",
  "AST_SIZEOF",
  "AST_IDENT",
  "AST_INT",
  "AST_CONDITIONAL",
  "AST_LABEL",
  "AST_CASE",
  "AST_DEFAULT",
  "AST_COMPOUND",
  "AST_EMPTY",
  "AST_IF",
  "AST_SWITCH",
  "AST_WHILE",
  "AST_DO_WHILE",
  "AST_FOR",
  "AST_GOTO",
  "AST_CONTINUE",
  "AST_BREAK",
  "AST_RETURN",
};

// TODO: consider doing variable length instead
typedef union {
  int64_t i64;
  double f64;
  uint16_t len; // ident, string
  uint16_t first_child;
} AstValue;

typedef struct {
  AstType type;
  uint16_t next_sibling;
  uint32_t start;
  AstValue value;
} AstNode;


uint16_t parse(const char *source, const Token *tokens, AstNode *ast_out);
void print_ast(const char *source, const AstNode *ast, uint16_t start, int indent_level);

#endif
