#ifndef INCLUDE_ISNT
#define INCLUDE_ISNT

#include "ast.h"
#include <stdint.h>

#define MAX_INSTRUCTIONS 512

typedef enum {
  INST_NONE,
  INST_INT,
  INST_ADD,
} InstType;

const char *INST_TYPE_NAME[] = {
  "none",
  "int",
  "add",
};

typedef struct {
  uint16_t type, a, b, c;
} Inst;

void codegen(const AstNode *ast, uint16_t ast_start, Inst insts[MAX_INSTRUCTIONS]);
void print_insts(const Inst *insts);

#endif
