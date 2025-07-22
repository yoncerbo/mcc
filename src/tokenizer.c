#include "tokens.h"
#include <stdint.h>
#include <assert.h>

TokenType ch2token[128 - 32] = {
  ['{' - 32] = TOK_LBRACE,
  ['}' - 32] = TOK_RBRACE,
  ['(' - 32] = TOK_LPAREN,
  [')' - 32] = TOK_RPAREN,
  [';' - 32] = TOK_SEMICOLON,
  // rest is TOK_NONE
};

void tokenize(const char *source, Token tokens_out[MAX_TOKENS]) {
  const char *ch = source;
  int len = 0;

  // for every token
  while (*ch) {
    while (*ch == ' ' || *ch == '\n' || *ch == '\t') ch++;

    if (*ch == '/' && ch[1] == '/') {
      ch += 2;
      while (*ch != '\n' || *ch != 0) ch++;
      ch++;
      continue;
    }

    if (*ch == '/' && ch[1] == '*') {
      ch += 2;
      while (*ch != '*' && ch[1] != '/') {
        assert(*ch); // TODO: error/warning EOF before end of comment
        ch++;
      }
      ch += 2;
      continue;
    }

    char *token_start = ch;
    TokenType tt = ch2token[*ch++ - 32];

    if (tt) {
      assert(len < MAX_TOKENS);
      tokens_out[len++] = (Token) {
        .type = tt,
        .len = ch - token_start,
      };
      continue;
    }
  }
}

