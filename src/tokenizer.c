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
  ['-' - 32] = TOK_MINUS,
  ['+' - 32] = TOK_PLUS,
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

    TokenType tt = ch2token[*ch - 32];
    // handle two character tokens

    if (tt) {
      assert(tokens_len < MAX_TOKENS);
      tokens_out[tokens_len++] = (Token) { tt, 1, ch - source };
      ch++;
      continue;
    }

    // strings
    // negative numbers, different literals
    if (IS_NUMERIC(*ch)) {
      const char *start = ch++;
      while (IS_NUMERIC(*ch)) ch++;
      assert(tokens_len < MAX_TOKENS);
      tokens_out[tokens_len++] = (Token){ TOK_DECIMAL, ch - start, start - source };
      continue;
    }

    if (*ch == '_' || IS_ALPHA(*ch)) {
      const char *start = ch++;
      while (*ch == '_' || IS_ALPHA(*ch) || IS_NUMERIC(*ch)) ch++;

      uint32_t len = ch - start;
      TokenType tt = TOK_IDENT;
      for (uint32_t i = 0; i < KEYWORD_COUNT; ++i) {
        if (keywords[i].str.len != len) continue;
        if (strncmp(keywords[i].str.ptr, start, len)) continue;
        tt = keywords[i].type;
        break;
      }
      assert(tokens_len < MAX_TOKENS);
      tokens_out[tokens_len++] = (Token){ tt, len, start - source };
      continue;
    }
  }
}

void print_tokens(Token *tokens) {
  while (tokens->type) {
    printf("(Token){ %s, %d, %d }\n", TOKEN_TYPE_STR[tokens->type], tokens->len, tokens->start);
    tokens++;
  }
}
