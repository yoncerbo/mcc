#ifndef INCLUDE_AST
#define INCLUDE_AST

#include <stdint.h>
#include "common.h"
#include "tokens.h"

#define MAX_AST_SIZE 512

typedef enum {
  AST_NONE,
  AST_IDENT,
  AST_INT,
  AST_DOT,
  AST_ARROW,
  AST_POST_INC,
  AST_POST_DEC,
  AST_PRE_INC,
  AST_PRE_DEC,
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

  AST_COUNT,
} AstExprType;

const char *AST_TYPE_STR[AST_COUNT] = {
  "AST_NONE",
  "AST_IDENT",
  "AST_INT",
  "AST_DOT",
  "AST_ARROW",
  "AST_POST_INC",
  "AST_POST_DEC",
  "AST_PRE_INC",
  "AST_PRE_DEC",
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
};

// TODO: consider doing variable length instead
typedef union {
  int64_t i64;
  double f64;
  uint16_t len; // ident, string
  uint16_t first_child;
} AstExprValue;

typedef struct {
  AstExprType type;
  uint16_t next_sibling;
  uint32_t start;
  AstExprValue value;
} AstExpr;


uint16_t parse(const char *source, const Token *tokens, AstExpr *ast_out);
void print_ast(const char *source, const AstExpr *ast, uint16_t start, int indent_level);

#endif
