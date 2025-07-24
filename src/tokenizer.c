#include "common.h"
#include "tokens.h"
#include <stdint.h>
#include <assert.h>
#include <string.h>

TokenType ch2token[128] = {
  ['-'] = TOK_MINUS, ['+'] = TOK_PLUS, ['<'] = TOK_LT,
  ['>'] = TOK_GT, ['='] = TOK_EQ, ['!'] = TOK_NOT,
  ['&'] = TOK_AND, ['*'] = TOK_STAR, ['/'] = TOK_SLASH,
  ['%'] = TOK_PERCENT, ['^'] = TOK_HAT, ['|'] = TOK_OR,

  ['.'] = TOK_DOT, ['~'] = TOK_TILDA, [':'] = TOK_COLON,
  [';'] = TOK_SEMICOLON, [','] = TOK_COMMA, ['('] = TOK_LPAREN,
  [')'] = TOK_RPAREN, ['{'] = TOK_LBRACE, ['}'] = TOK_RBRACE,
  ['['] = TOK_LSQUARE, [']'] = TOK_RSQUARE, ['?'] = TOK_QUESTION,
};

typedef struct {
  char ch;
  TokenType type;
} OptionTokenType;

OptionTokenType multiple_tokens[TOK_ARROW][2] = {
  [TOK_MINUS]   = {{ '>', TOK_ARROW },      { '=', TOK_MINUS_EQ }},
  [TOK_PLUS]    = {{ '+', TOK_DPLUS },      { '=', TOK_PLUS_EQ }},
  [TOK_LT]      = {{ '<', TOK_LSFT },       { '=', TOK_LE }},
  [TOK_GT]      = {{ '>', TOK_RSFT },       { '=', TOK_GE }},
  [TOK_EQ]      = {{ '=', TOK_DEQ },        {0}},
  [TOK_NOT]     = {{ '=', TOK_NEQ },        {0}},
  [TOK_AND]     = {{ '&', TOK_DAND },       { '=', TOK_AND_EQ }},
  [TOK_STAR]    = {{ '=', TOK_STAR_EQ },    {0}},
  [TOK_SLASH]   = {{ '=', TOK_SLASH_EQ },   {0}},
  [TOK_PERCENT] = {{ '=', TOK_PERCENT_EQ }, {0}},
  [TOK_HAT]     = {{ '=', TOK_DPLUS },      {0}},
  [TOK_OR]      = {{ '|', TOK_DOR },        { '=', TOK_DEQ }},

};

// A very simple solution for now 
// https://rgambord.github.io/c99-doc/sections/8/1/2/index.html
// !IMPORTANT has to be the same order is in TokenType enum
Str keywords[] = {
  STR("auto"), STR("enum"), STR("restrict"), STR("unsigned"), STR("break"),
  STR("extern"), STR("return"), STR("void"), STR("case"), STR("float"),
  STR("short"), STR("volatile"), STR("char"), STR("for"), STR("signed"),
  STR("while"), STR("const"), STR("goto"), STR("sizeof"), STR("_Bool"),
  STR("continue"), STR("if"), STR("static"), STR("_Complex"), STR("default"),
  STR("inline"), STR("struct"), STR("_Imaginary"), STR("do"), STR("int"),
  STR("switch"), STR("double"), STR("long"), STR("typedef"), STR("else"),
  STR("register"), STR("union"),
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

    TokenType tt = ch2token[(uint8_t)*ch];

    if (tt) {
      uint32_t len = 1;
      // TODO: How to make it better?
      if (tt < TOK_ARROW) {
        if (ch[1] && ch[1] == multiple_tokens[tt][0].ch) {
          tt = multiple_tokens[tt][0].type;
          len = 2;
        } else if (ch[1] && ch[1] == multiple_tokens[tt][1].ch) {
          tt = multiple_tokens[tt][1].type;
          len = 2;
        }
      }
      if (tt == TOK_MINUS && ch[1] == '-') {
        tt = TOK_DMINUS;
        len = 2;
      } else if (tt == TOK_LSFT && ch[2] == '=') {
        tt = TOK_LSFT_EQ;
        len = 3;
      } else if (tt == TOK_RSFT && ch[2] == '=') {
        tt = TOK_RSF_EQ;
        len = 3;
      }
      assert(tokens_len < MAX_TOKENS);
      tokens_out[tokens_len++] = (Token){ tt, len, ch - source };
      ch += len;
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
        if (keywords[i].len != len) continue;
        if (strncmp(keywords[i].ptr, start, len)) continue;
        tt = TOK_AUTO + i;
        break;
      }
      assert(tokens_len < MAX_TOKENS);
      tokens_out[tokens_len++] = (Token){ tt, len, start - source };
      continue;
    }
  }
}

void print_tokens(const Token *tokens) {
  while (tokens->type) {
    printf("% 3d:%02d %s\n", tokens->start, tokens->len, TOKEN_TYPE_STR[tokens->type]);
    tokens++;
  }
}
