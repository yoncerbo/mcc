
#include "ast.h"
#include "inst.h"
#include <stdint.h>
#include <assert.h>

typedef struct {
  const AstNode *ast;
  Inst *insts;
  uint16_t inst_len;
} Codegen;

static inline uint16_t Codegen_inst(Codegen *c, Inst inst) {
  assert(c->inst_len < MAX_INSTRUCTIONS);
  uint16_t index = c->inst_len++;
  c->insts[index] = inst;
  return index;
}

uint16_t Codegen_value(Codegen *c, uint16_t start) {
  AstNode expr = c->ast[start];
  uint16_t a, b;
  switch (expr.type) {
    case AST_INT:
      assert(expr.value.i64 <= INT32_MAX);
      uint16_t low = expr.value.i64;
      uint16_t high = expr.value.i64 >> 16;
      // TODO: a quick fix, think about a better way, maybe variable length?
      return Codegen_inst(c, (Inst){ INST_INT, low, high, 0 });
    case AST_ADD:
      uint16_t left = expr.value.first_child;
      a = Codegen_value(c, left);
      b = Codegen_value(c, c->ast[left].next_sibling);
      return Codegen_inst(c, (Inst){ INST_ADD, a, b, 0 });
    default:
      assert(0);
  }
}

void codegen(const AstNode *ast, uint16_t ast_start, Inst insts[MAX_INSTRUCTIONS]) {
  Codegen c = {
    .ast = ast,
    .insts = insts,
  };
  Codegen_value(&c, ast_start);
}

void print_insts(const Inst *insts) {
  int i = 0;
  while (insts->type) {
    printf("% 3d %s ", i, INST_TYPE_NAME[insts->type]);
    switch(insts->type) {
      case INST_INT:
        printf("%d\n", insts->a | (insts->b << 16));
        break;
      case INST_ADD:
        printf("t%d, t%d\n", insts->a, insts->b);
        break;
      default:
        putchar(10);
    }
    insts++;
    i++;
  }
}

