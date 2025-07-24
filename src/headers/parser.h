#ifndef INCLUDE_PARSER
#define INCLUDE_PARSER

#include "common.h"
#include "ast.h"

#define MAX_AST_SIZE 512
#define MAX_VARIABLES 256
#define MAX_SCOPES 64
#define MAX_LABELS 64

typedef struct {
  Str labels[MAX_LABELS];
  Str vars[MAX_VARIABLES];
  uint16_t scopes[MAX_SCOPES];
  const char *source;
  const Token *tokens;
  AstNode *ast_out;
  uint16_t pos;
  uint16_t ast_size;
  uint16_t var_size;
  uint16_t labels_size;
  uint8_t scope;
} Parser;

typedef uint16_t AstId;
typedef uint16_t VarId;
typedef uint16_t LabelId;

AstId parse(const char *source, const Token *tokens, AstNode *ast_out, Parser *p);
void print_ast(Parser *p, uint16_t node, int indent_level);

AstId Parser_parse_expression(Parser *p);
AstId Parser_parse_unary(Parser *p);
AstId Parser_parse_conditional(Parser *p, uint16_t left);
AstId Parser_parse_declaration(Parser *p);
AstId Parser_parse_statement(Parser *p);
AstId Parser_parse_block(Parser *p);
AstId Parser_create_expr(Parser *p, AstNode expr);
AstId Parser_create_ident(Parser *p, Token source);

LabelId Parser_push_label(Parser *p, Str name);
LabelId Parser_resolve_label(Parser *p, Str name);
VarId Parser_push_var(Parser *p, Str name);
VarId Parser_resolve_var(Parser *p, Str name);
void Parser_push_scope(Parser *p);
void Parser_pop_scope(Parser *p);

#endif
