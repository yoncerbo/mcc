#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#include "tokenizer.c"
#include "tokens.h"

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

  Token *tokens = malloc(sizeof(*tokens) * MAX_TOKENS);
  tokenize("(;)", tokens);

  Token *tok = tokens;
  uint32_t start = 0;
  while (tok->type) {
    printf("type=%s, start=%d, len=%d\n", TOKEN_TYPE_STR[tok->type], start, tok->len);
    start += tok->len;
    tok++;
  }

  return 0;
}
