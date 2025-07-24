#include "ast.h"
#include "parser.h"

uint16_t Parser_parse_declaration(Parser *p) {
  Token tok = p->tokens[p->pos];
  uint16_t value = 0;
  switch (tok.type) {
    case TOK_INT:
      p->pos++;
      Token ident = p->tokens[p->pos++];
      assert(ident.type == TOK_IDENT);
      Str name = (Str){ &p->source[ident.start], ident.len };
      if(p->tokens[p->pos].type == TOK_EQ) {
        p->pos++;
        value = Parser_parse_expression(p);
      }
      assert(p->tokens[p->pos++].type == TOK_SEMICOLON);
      VarId var = Parser_push_var(p, name);
      return Parser_create_expr(p, (AstNode){
        .type = AST_DECL,
        .start = ident.start, // TODO: store the ident's start elsewhere
        .value.decl.value = value,
        .value.decl.var = var,
      });
      break;
    default:
      return Parser_parse_statement(p);
  }
}

