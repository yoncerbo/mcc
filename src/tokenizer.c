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
Str keywords[] = {
  [TOK_AUTO - KEYWORDS_START] = STR("auto"),
  [TOK_ENUM - KEYWORDS_START] = STR("enum"),
  [TOK_RESTRICT - KEYWORDS_START] = STR("restrict"),
  [TOK_UNSIGNED - KEYWORDS_START] = STR("unsigned"),
  [TOK_BREAK - KEYWORDS_START] = STR("break"),
  [TOK_EXTERN - KEYWORDS_START] = STR("extern"),
  [TOK_RETURN - KEYWORDS_START] = STR("return"),
  [TOK_VOID - KEYWORDS_START] = STR("void"),
  [TOK_CASE - KEYWORDS_START] = STR("case"),
  [TOK_FLOAT - KEYWORDS_START] = STR("float"),
  [TOK_SHORT - KEYWORDS_START] = STR("short"),
  [TOK_VOLATILE - KEYWORDS_START] = STR("volatile"),
  [TOK_CHAR - KEYWORDS_START] = STR("char"),
  [TOK_FOR - KEYWORDS_START] = STR("for"),
  [TOK_SIGNED - KEYWORDS_START] = STR("signed"),
  [TOK_WHILE - KEYWORDS_START] = STR("while"),
  [TOK_CONST - KEYWORDS_START] = STR("const"),
  [TOK_GOTO - KEYWORDS_START] = STR("goto"),
  [TOK_SIZEOF - KEYWORDS_START] = STR("sizeof"),
  [TOK_BOOL - KEYWORDS_START] = STR("_Bool"),
  [TOK_CONTINUE - KEYWORDS_START] = STR("continue"),
  [TOK_IF - KEYWORDS_START] = STR("if"),
  [TOK_STATIC - KEYWORDS_START] = STR("static"),
  [TOK_COMPLEX - KEYWORDS_START] = STR("_Complex"),
  [TOK_DEFAULT - KEYWORDS_START] = STR("default"),
  [TOK_INLINE - KEYWORDS_START] = STR("inline"),
  [TOK_STRUCT - KEYWORDS_START] = STR("struct"),
  [TOK_IMAGINARY - KEYWORDS_START] = STR("_Imaginary"),
  [TOK_DO - KEYWORDS_START] = STR("do"),
  [TOK_INT - KEYWORDS_START] = STR("int"),
  [TOK_SWITCH - KEYWORDS_START] = STR("switch"),
  [TOK_DOUBLE - KEYWORDS_START] = STR("double"),
  [TOK_LONG - KEYWORDS_START] = STR("long"),
  [TOK_TYPEDEF - KEYWORDS_START] = STR("typedef"),
  [TOK_ELSE - KEYWORDS_START] = STR("else"),
  [TOK_REGISTER - KEYWORDS_START] = STR("register"),
  [TOK_UNION - KEYWORDS_START] = STR("union"),
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
      while (*ch != '\n') {
        if (*ch == '\0') return;
        ch++;
      }
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
        tt = KEYWORDS_START + i;
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
