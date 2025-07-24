#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#include "ast.h"
#include "common.h"
#include "inst.h"
#include "tokenizer.c"
#include "tokens.h"
#include "parser.c"
#include "codegen.c"
// #include "assembly.c"

int main(int argc, const char *argv[]) {
  assert(argc == 2);
  const char *filename = argv[1];
  printf("Reading file '%s'\n", filename);

  int fd = open(filename, O_RDONLY);
  assert(fd >= 0);

  struct stat st;
  assert(fstat(fd, &st) >= 0);

  char *file = mmap(0, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
  assert(file != MAP_FAILED);

  DEBUGS(TOKEN_TYPE_STR[TOK_PLUS]);
  printf("\nTokenizing:\n");
  Token *tokens = malloc(sizeof(*tokens) * MAX_TOKENS);
  tokenize(file, tokens);
  print_tokens(tokens);

  printf("\nParsing:\n");
  Parser p;
  AstNode *ast = malloc(sizeof(*ast) * MAX_AST_SIZE);
  uint16_t index = parse(file, tokens, ast, &p);
  print_ast(&p, index, 0);

  printf("\nCodegen:\n");
  Inst *insts = malloc(sizeof(*insts) * MAX_INSTRUCTIONS);
  codegen(ast, index, insts);
  print_insts(insts);

  // printf("\nGenerating assembly:\n");
  // generate_assembly(insts);

  return 0;
}
