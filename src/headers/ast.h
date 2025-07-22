#ifndef INCLUDE_AST
#define INCLUDE_AST

#include <stdint.h>
#include "common.h"
#include "tokens.h"

#define MAX_AST_SIZE 512

typedef enum {
  AST_NONE,
  AST_ZERO,
  AST_INT,

  AST_COUNT,
} AstExprType;

const char *AST_TYPE_STR[AST_COUNT] = {
  "AST_NONE",
  "AST_ZERO",
  "AST_INT",
};

// TODO: consider doing variable length instead
typedef union {
  int64_t i64;
  double f64;
  Span str;
  uint16_t children;
} AstExprValue;

typedef struct {
  AstExprType type;
  uint16_t len;
  uint32_t start;
  AstExprValue value;
} AstExpr;


void parse(const char *source, const Token *tokens, AstExpr *ast_out);
void print_ast(const AstExpr *ast);

#endif
