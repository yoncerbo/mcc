#ifndef INCLUDE_AST
#define INCLUDE_AST

#include <stdint.h>
#include "common.h"
#include "tokens.h"

typedef enum {
  AST_NONE,

  // Binary operations
  AST_MUL, AST_DIV, AST_MOD, AST_ADD, AST_SUB,
  AST_LSFT, AST_RSFT, AST_LT, AST_LE, AST_GT,
  AST_GE, AST_EQ, AST_NE, AST_BAND, AST_BXOR,
  AST_BOR, AST_LAND, AST_LOR,

  // Assignment operators
  AST_ASS, AST_ASS_MUL, AST_ASS_DIV, AST_ASS_MOD,
  AST_ASS_ADD, AST_ASS_SUB, AST_ASS_LSFT, AST_ASS_RSFT,
  AST_ASS_AND, AST_ASS_XOR, AST_ASS_OR,

  // Prefix expressions
  AST_INDEX, AST_CALL, AST_DOT, AST_ARROW,
  AST_POST_INC, AST_POST_DEC,

  // Postfix expressions
  AST_PRE_INC, AST_PRE_DEC, AST_ADDR, AST_DEREF,
  AST_PLUS, AST_MINUS, AST_NEG, AST_NOT, AST_SIZEOF,

  // Other expressions
  AST_IDENT, AST_INT, AST_CONDITIONAL, AST_VAR,

  // Statements
  AST_LABEL, AST_CASE, AST_DEFAULT, AST_COMPOUND,
  AST_EMPTY, AST_IF, AST_SWITCH, AST_WHILE,
  AST_DO_WHILE, AST_FOR, AST_GOTO, AST_CONTINUE,
  AST_BREAK, AST_RETURN,

  // Declarations
  AST_DECL,

  AST_COUNT,
} AstType;

const char *AST_TYPE_STR[AST_COUNT] = {
  "AST_NONE", "AST_MUL", "AST_DIV", "AST_MOD", "AST_ADD",
  "AST_SUB", "AST_LSFT", "AST_RSFT", "AST_LT", "AST_LE",
  "AST_GT", "AST_GE", "AST_EQ", "AST_NE", "AST_BAND",
  "AST_BXOR", "AST_BOR", "AST_LAND", "AST_LOR", "AST_ASS",
  "AST_ASS_MUL", "AST_ASS_DIV", "AST_ASS_MOD", "AST_ASS_ADD", "AST_ASS_SUB",
  "AST_ASS_LSFT", "AST_ASS_RSFT", "AST_ASS_AND", "AST_ASS_XOR", "AST_ASS_OR",
  "AST_INDEX", "AST_CALL", "AST_DOT", "AST_ARROW", "AST_POST_INC",
  "AST_POST_DEC", "AST_PRE_INC", "AST_PRE_DEC", "AST_ADDR", "AST_DEREF",
  "AST_PLUS", "AST_MINUS", "AST_NEG", "AST_NOT", "AST_SIZEOF",
  "AST_IDENT", "AST_INT", "AST_CONDITIONAL", "AST_VAR", "AST_LABEL",
  "AST_CASE", "AST_DEFAULT", "AST_COMPOUND", "AST_EMPTY", "AST_IF",
  "AST_SWITCH", "AST_WHILE", "AST_DO_WHILE", "AST_FOR", "AST_GOTO",
  "AST_CONTINUE", "AST_BREAK", "AST_RETURN", "AST_DECL",
};

// TODO: consider doing variable length instead
typedef union {
  int64_t i64;
  double f64;
  uint16_t len; // ident, string
  uint16_t first_child;
  uint16_t var;
  uint16_t label;
  struct AstValueDecl {
    uint16_t first_child, var_start, var_count;
  } decl;
} AstValue;

typedef struct {
  AstType type;
  uint16_t next_sibling;
  uint32_t start;
  AstValue value;
} AstNode;

// TODO: use one byte for data type and flags, where possible
typedef enum {
  DATA_NONE,
  DATA_VOID,
  DATA_CHAR,
  DATA_FLOAT,
  DATA_DOUBLE,
  DATA_BOOL,
  DATA_COMPLEX,

  DATA_UCHAR,
  DATA_INT,
  DATA_UINT,
  DATA_SHORT_INT,
  DATA_SHORT_UINT,
  DATA_LONG_INT,
  DATA_LONG_UINT,
  DATA_LONG_LONG_INT,
  DATA_LONG_LONG_UINT,
  DATA_LONG_DOUBLE,
  DATA_STRUCT,
  DATA_UNION,
  DATA_ENUM,

  DATA_COUNT,
} DataType;

const char *DATA_TYPE_TO_STR[DATA_COUNT] = {
  "none", "void", "char", "float", "double",
  "bool", "complex", "uchar", "int", "uint",
  "sint", "suint", "lint", "luint", "llint",
  "lluint", "ldouble", "struct", "union", "enum",
};

typedef enum {
  // 4 options - 2 bits
  STORAGE_EXTERN,
  STORAGE_AUTO, // default
  STORAGE_STATIC,
  STORAGE_REGISTER,
  // this is only used during parsing
  STORAGE_TYPEDEF,
  STORAGE_NONE,
  STORAGE_COUNT,
} StorageType;

const char *STORAGE_TO_STR[STORAGE_COUNT] = {
  "extern", "auto", "static", "register", "typedef", "none",
};

// TODO: reorder the tokens in the definition
AstType tok2operation[TOK_IDENT] = {
  // For postfix expression
  [TOK_LSQUARE] = AST_INDEX,
  [TOK_LPAREN] = AST_CALL,
  [TOK_DOT] = AST_DOT,
  [TOK_ARROW] = AST_ARROW,
  [TOK_DPLUS] = AST_POST_INC,
  [TOK_DMINUS] = AST_POST_DEC,

  // For binary operators
  [TOK_STAR] = AST_MUL, [TOK_SLASH] = AST_DIV, [TOK_PERCENT] = AST_MUL,
  [TOK_PLUS] = AST_ADD, [TOK_MINUS] = AST_SUB, [TOK_LSFT] = AST_LSFT,
  [TOK_RSFT] = AST_RSFT, [TOK_LT] = AST_LT, [TOK_LE] = AST_LE,
  [TOK_GT] = AST_GT, [TOK_GE] = AST_GE, [TOK_DEQ] = AST_EQ,
  [TOK_NEQ] = AST_NE, [TOK_AND] = AST_BAND, [TOK_HAT] = AST_BXOR,
  [TOK_OR] = AST_BOR, [TOK_DAND] = AST_LAND, [TOK_DOR] = AST_LOR,

  // For assignments
  [TOK_EQ] = AST_ASS, [TOK_STAR_EQ] = AST_ASS_MUL, [TOK_SLASH_EQ] = AST_ASS_DIV,
  [TOK_PERCENT_EQ] = AST_ASS_MOD, [TOK_PLUS_EQ] = AST_ASS_ADD, [TOK_MINUS_EQ] = AST_ASS_SUB,
  [TOK_LSFT_EQ] = AST_ASS_LSFT, [TOK_RSF_EQ] = AST_ASS_RSFT, [TOK_AND_EQ] = AST_ASS_AND,
  [TOK_HAT_EQ] = AST_ASS_XOR, [TOK_OR_EQ] = AST_ASS_OR,
};

uint8_t op2precedence[AST_ASS] = {
  [AST_MUL] = 10, [AST_DIV] = 10, [AST_MOD] = 10,
  [AST_ADD] = 9, [AST_SUB] = 9, [AST_LSFT] = 8,
  [AST_RSFT] = 8, [AST_LT] = 7, [AST_LE] = 7,
  [AST_GT] = 7, [AST_GE] = 7, [AST_EQ] = 6,
  [AST_NE] = 6, [AST_BAND] = 5, [AST_BXOR] = 4,
  [AST_BOR] = 3, [AST_LAND] = 2, [AST_LOR] = 1,
};

#endif
