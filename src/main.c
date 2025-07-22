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
#include "tokenizer.c"
#include "tokens.h"
#include "parser.c"

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
  tokenize("-=", tokens);
  print_tokens(tokens);

  printf("\nParsing:\n");
  AstExpr *ast = malloc(sizeof(*ast) * MAX_AST_SIZE);
  parse(file, tokens, ast);
  print_ast(ast);

  return 0;
}
