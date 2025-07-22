#include "common.h"
#include "tokens.h"
#include <stdint.h>
#include <assert.h>
#include <string.h>

#define IS_NUMERIC(ch) ((ch) >= '0' && (ch) <= '9')
#define IS_ALPHA(ch) \
    (((ch) >= 'a' && (ch) <= 'z') || ((ch) >= 'A' && (ch) <= 'Z'))

TokenType ch2token[128 - 32] = {
  ['{' - 32] = TOK_LBRACE,
  ['}' - 32] = TOK_RBRACE,
  ['(' - 32] = TOK_LPAREN,
  [')' - 32] = TOK_RPAREN,
  [';' - 32] = TOK_SEMICOLON,
  // rest is TOK_NONE
};

typedef struct {
  Str str;
  TokenType type;
} Keyword;

// A very simple solution for now 
Keyword keywords[] = {
  { STR("int"), TOK_INT },
  { STR("return"), TOK_RETURN },
  { STR("void"), TOK_VOID },
};
const uint32_t KEYWORD_COUNT = sizeof(keywords) / sizeof(*keywords);

void tokenize(const char *source, Token tokens_out[MAX_TOKENS]) {
  const char *ch = source;
  int tokens_len = 0;

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

    const char *token_start = ch;
    TokenType tt = ch2token[*ch - 32];

    if (tt) {
      ch++;
      assert(tokens_len < MAX_TOKENS);
      tokens_out[tokens_len++] = (Token) { tt, ch - token_start };
      continue;
    }

    // strings
    if (*ch == '_' || IS_ALPHA(*ch)) {
      ch++;
      while (*ch == '_' || IS_ALPHA(*ch) || IS_NUMERIC(*ch)) ch++;

      uint32_t len = ch - token_start;
      TokenType tt = TOK_IDENT;
      for (uint32_t i = 0; i < KEYWORD_COUNT; ++i) {
        if (keywords[i].str.len != len) continue;
        if (strncmp(keywords[i].str.ptr, token_start, len)) continue;
        tt = keywords[i].type;
        break;
      }
      assert(tokens_len < MAX_TOKENS);
      tokens_out[tokens_len++] = (Token){ tt, len };
      continue;
    }
  }
}

