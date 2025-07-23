#include "assembly.h"
#include "inst.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>

#define REGISTER_COUNT 8
const char *REGISTERS[REGISTER_COUNT] = { "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15" };

typedef struct {
  const Inst *insts;
  uint16_t pos;
  uint8_t free_registers[REGISTER_COUNT];
  uint8_t free_registers_len;
  uint8_t inst2reg[MAX_INSTRUCTIONS];
  // TODO: what's a good name for this?
} Generator;

uint8_t alloc_register(Generator *g) {
  assert(g->free_registers_len);
  return g->free_registers[--g->free_registers_len];
}

void free_register(Generator *g, uint8_t reg) {
  assert(g->free_registers_len < REGISTER_COUNT);
  g->free_registers[g->free_registers_len++] = reg;
}

uint8_t generate_inst(Generator *g) {
  Inst inst = g->insts[g->pos];
  switch (inst.type) {
    case INST_INT:
      uint8_t reg = alloc_register(g);
      printf("  movq %s, %d\n", REGISTERS[reg], inst.a | (inst.b << 16));
      g->inst2reg[g->pos] = reg;
      g->pos++;
      break;
    case INST_ADD:
      uint8_t left = g->inst2reg[inst.a];
      uint8_t right = g->inst2reg[inst.b];
      free_register(g, right);
      printf("  addq %s, %s\n", REGISTERS[left], REGISTERS[right]);
      g->inst2reg[g->pos] = left;
      g->pos++;
    default:
      break;
  }
}

void generate_assembly(const Inst *insts) {
  Generator g = {
    .free_registers_len = REGISTER_COUNT,
    .insts = insts,
  };
  for (int i = 0; i < REGISTER_COUNT; ++i) g.free_registers[i] = i;
  printf("main:\n");
  for (int i = 0; insts[i].type; ++i) {
    generate_inst(&g);
  }
  printf("  ret\n");
}

