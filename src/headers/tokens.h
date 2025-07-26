#ifndef INCLUDE_TOKENS
#define INCLUDE_TOKENS

#include <stdint.h>

#define MAX_TOKENS 256

#define IS_NUMERIC(ch) ((ch) >= '0' && (ch) <= '9')
#define IS_ALPHA(ch) \
    (((ch) >= 'a' && (ch) <= 'z') || ((ch) >= 'A' && (ch) <= 'Z'))

typedef enum {
  TOK_NONE,

  // Those tokens require further processing
  // - -> -= --
  // + ++ +=
  // < << <= <<=
  // > >> >= >>=
  // = ==
  // ! !=
  // & && &=
  // * *=
  // / /=
  // % %=
  // ^ ^=
  // | || |=
  TOK_MINUS, TOK_PLUS, TOK_LT, TOK_GT,
  TOK_EQ, TOK_NOT, TOK_AND, TOK_STAR,
  TOK_SLASH, TOK_PERCENT, TOK_HAT, TOK_OR,

  TOK_ARROW, TOK_MINUS_EQ, TOK_DMINUS, TOK_DPLUS,
  TOK_PLUS_EQ, TOK_LSFT, TOK_LE, TOK_LSFT_EQ, 
  TOK_RSFT, TOK_GE, TOK_RSF_EQ, TOK_DEQ, 
  TOK_NEQ, TOK_DAND, TOK_AND_EQ, TOK_STAR_EQ,
  TOK_SLASH_EQ, TOK_PERCENT_EQ, TOK_HAT_EQ, TOK_DOR,
  TOK_OR_EQ,

  TOK_DOT, TOK_TILDA, TOK_COLON, TOK_SEMICOLON,
  TOK_COMMA, TOK_LPAREN, TOK_RPAREN, TOK_LBRACE,
  TOK_RBRACE, TOK_LSQUARE, TOK_RSQUARE, TOK_QUESTION,

  TOK_IDENT,
  TOK_DECIMAL,

#define KEYWORDS_START TOK_BREAK
  // Other keywords
  TOK_BREAK, TOK_FOR, TOK_WHILE, TOK_GOTO, TOK_SIZEOF,
  TOK_CONTINUE, TOK_IF, TOK_DEFAULT, TOK_IMAGINARY, TOK_DO,
  TOK_RETURN, TOK_CASE, TOK_ELSE, TOK_SWITCH,

#define DECL_SPEC_START TOK_EXTERN
  // Storage specifier, same order as in StorageType
  TOK_EXTERN, TOK_AUTO, TOK_STATIC, TOK_REGISTER, TOK_TYPEDEF,

  // FLags
  TOK_CONST, TOK_RESTRICT, TOK_VOLATILE, TOK_INLINE,

  // Base type
  TOK_VOID, TOK_CHAR, TOK_FLOAT, TOK_DOUBLE,
  TOK_BOOL, TOK_COMPLEX, _TOK_PADDING, TOK_INT,

  // Struct and union
  TOK_STRUCT, TOK_UNION,

  // Other
  TOK_ENUM, TOK_UNSIGNED, TOK_SHORT, TOK_SIGNED, TOK_LONG,

  TOK_COUNT,
} TokenType;

// TODO: tokens have been reordered, fix it
const char *TOKEN_TYPE_STR[TOK_COUNT] = {
  "TOK_NONE",

  "TOK_MINUS", "TOK_PLUS", "TOK_LT", "TOK_GT",
  "TOK_EQ", "TOK_NOT", "TOK_AND", "TOK_STAR",
  "TOK_SLASH", "TOK_PERCENT", "TOK_HAT", "TOK_OR",

  "TOK_ARROW", "TOK_MINUS_EQ", "TOK_DMINUS", "TOK_DPLUS",
  "TOK_PLUS_EQ", "TOK_LSFT", "TOK_LE", "TOK_LSFT_EQ",
  "TOK_RSFT", "TOK_GE", "TOK_RSF_EQ", "TOK_DEQ",
  "TOK_NEQ", "TOK_DAND", "TOK_AND_EQ", "TOK_STAR_EQ",
  "TOK_SLASH_EQ", "TOK_PERCENT_EQ", "TOK_HAT_EQ", "TOK_DOR",
  "TOK_OR_EQ",

  "TOK_DOT", "TOK_TILDA", "TOK_COLON", "TOK_SEMICOLON",
  "TOK_COMMA", "TOK_LPAREN", "TOK_RPAREN", "TOK_LBRACE",
  "TOK_RBRACE", "TOK_LSQUARE", "TOK_RSQUARE", "TOK_QUESTION",

  "TOK_IDENT",
  "TOK_DECIMAL",

  "TOK_BREAK", "TOK_FOR", "TOK_WHILE", "TOK_GOTO", "TOK_SIZEOF",
  "TOK_CONTINUE", "TOK_IF", "TOK_DEFAULT", "TOK_IMAGINARY", "TOK_DO",
  "TOK_RETURN", "TOK_CASE", "TOK_ELSE", "TOK_SWITCH",

  "TOK_EXTERN", "TOK_AUTO", "TOK_STATIC", "TOK_REGISTER", "TOK_TYPEDEF",
  "TOK_CONST", "TOK_RESTRICT", "TOK_VOLATILE", "TOK_INLINE",
  "TOK_VOID", "TOK_CHAR", "TOK_FLOAT", "TOK_DOUBLE",
  "TOK_BOOL", "TOK_COMPLEX", "_TOK_PADDING", "TOK_INT",
  "TOK_STRUCT", "TOK_UNION",
  "TOK_ENUM", "TOK_UNSIGNED", "TOK_SHORT", "TOK_SIGNED", "TOK_LONG",

};

typedef struct {
  TokenType type;
  uint16_t len;
  uint32_t start;
} Token;

void tokenize(const char *source, Token tokens_out[MAX_TOKENS]);
void print_tokens(const Token *tokens);

#endif

