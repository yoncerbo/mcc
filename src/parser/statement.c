#include "ast.h"
#include "parser.h"

uint16_t Parser_parse_block(Parser *p) {
  uint16_t first = 0, last = 0;
  while(p->tokens[p->pos].type != TOK_RBRACE) {
    assert(p->tokens[p->pos].type);
    uint16_t elem = Parser_parse_declaration(p);
    if (!elem) continue; // declaration migth not create any nodes
    if (!first) first = elem;
    else p->ast_out[last].next_sibling = elem;
    last = elem;
  }
  p->pos++;
  return first;
}

// TODO: slim it down, make lookup table
uint16_t Parser_parse_statement(Parser *p) {
  uint16_t inner = 0;
  uint16_t last = 0;
  Token tok = p->tokens[p->pos];
  switch (tok.type) {
    case TOK_IDENT:
      if (p->tokens[p->pos + 1].type != TOK_COLON) break;
      p->pos += 2;
      uint16_t label = Parser_push_label(p, (Str){ &p->source[tok.start], tok.len });
      return Parser_create_expr(p, (AstNode){
        .type = AST_LABEL,
        .start = tok.start,
        .value.label = label,
      });
    case TOK_CASE:
      p->pos++;
      // constant expressions
      // https://rgambord.github.io/c99-doc/sections/6/6/index.html
      inner = Parser_parse_conditional(p, Parser_parse_unary(p));
      assert(p->tokens[p->pos++].type == TOK_COLON);
      p->ast_out[inner].next_sibling = Parser_parse_statement(p);
      return Parser_create_expr(p, (AstNode){
        .type = AST_CASE,
        .start = tok.start,
        .value.first_child = inner,
      });
    case TOK_DEFAULT:
      p->pos++;
      assert(p->tokens[p->pos++].type == TOK_COLON);
      inner = Parser_parse_statement(p);
      return Parser_create_expr(p, (AstNode){
        .type = AST_DEFAULT,
        .start = tok.start,
        .value.first_child = inner,
      });
    case TOK_LBRACE:
      Parser_push_scope(p);
      p->pos++;
      inner = Parser_parse_block(p);
      Parser_pop_scope(p);
      return Parser_create_expr(p, (AstNode){
        .type = AST_COMPOUND,
        .start = tok.start,
        .value.first_child = inner,
      });
    case TOK_SEMICOLON:
      p->pos++;
      return Parser_create_expr(p, (AstNode){
        .type = AST_EMPTY,
        .start = tok.start,
        .value.first_child = 0,
      });
    case TOK_IF:
      p->pos++;
      assert(p->tokens[p->pos++].type == TOK_LPAREN);
      inner = Parser_parse_expression(p);
      assert(p->tokens[p->pos++].type == TOK_RPAREN);
      last = Parser_parse_statement(p);
      p->ast_out[inner].next_sibling = last;
      if (p->tokens[p->pos].type == TOK_ELSE) {
        p->pos++;
        p->ast_out[last].next_sibling = Parser_parse_statement(p);
      }
      return Parser_create_expr(p, (AstNode){
        .type = AST_IF,
        .start = tok.start,
        .value.first_child = inner,
      });
    case TOK_SWITCH:
      p->pos++;
      assert(p->tokens[p->pos++].type == TOK_LPAREN);
      inner = Parser_parse_expression(p);
      assert(p->tokens[p->pos++].type == TOK_RPAREN);
      last = Parser_parse_statement(p);
      p->ast_out[inner].next_sibling = last;
      return Parser_create_expr(p, (AstNode){
        .type = AST_SWITCH,
        .start = tok.start,
        .value.first_child = inner,
      });
    case TOK_WHILE:
      p->pos++;
      assert(p->tokens[p->pos++].type == TOK_LPAREN);
      inner = Parser_parse_expression(p);
      assert(p->tokens[p->pos++].type == TOK_RPAREN);
      last = Parser_parse_statement(p);
      p->ast_out[inner].next_sibling = last;
      return Parser_create_expr(p, (AstNode){
        .type = AST_WHILE,
        .start = tok.start,
        .value.first_child = inner,
      });
    case TOK_DO:
      p->pos++;
      inner = Parser_parse_statement(p);
      assert(p->tokens[p->pos++].type == TOK_WHILE);
      assert(p->tokens[p->pos++].type == TOK_LPAREN);
      last = Parser_parse_expression(p);
      assert(p->tokens[p->pos++].type == TOK_RPAREN);
      assert(p->tokens[p->pos++].type == TOK_SEMICOLON);
      p->ast_out[inner].next_sibling = last;
      return Parser_create_expr(p, (AstNode){
        .type = AST_DO_WHILE,
        .start = tok.start,
        .value.first_child = inner,
      });
    case TOK_FOR:
      p->pos++;
      assert(p->tokens[p->pos++].type == TOK_LPAREN);
      if (p->tokens[p->pos].type == TOK_SEMICOLON) {
        inner = Parser_create_expr(p, (AstNode){
          .type = AST_EMPTY,
          .start = p->tokens[p->pos].start,
          .value.first_child = 0,
        });
        p->pos++;
      } else {
        inner = Parser_parse_expression(p);
        assert(p->tokens[p->pos++].type == TOK_SEMICOLON);
      }
      if (p->tokens[p->pos].type == TOK_SEMICOLON) {
        last = Parser_create_expr(p, (AstNode){
          .type = AST_EMPTY,
          .start = p->tokens[p->pos].start,
          .value.first_child = 0,
        });
        p->pos++;
      } else {
        last = Parser_parse_expression(p);
        assert(p->tokens[p->pos++].type == TOK_SEMICOLON);
      }
      p->ast_out[inner].next_sibling = last;
      uint16_t elem;
      if (p->tokens[p->pos].type == TOK_RPAREN) {
        elem = Parser_create_expr(p, (AstNode){
          .type = AST_EMPTY,
          .start = p->tokens[p->pos].start,
          .value.first_child = 0,
        });
        p->pos++;
      } else {
        elem = Parser_parse_expression(p);
        assert(p->tokens[p->pos++].type == TOK_RPAREN);
      }
      p->ast_out[last].next_sibling = elem;
      p->ast_out[elem].next_sibling = Parser_parse_statement(p);
      return Parser_create_expr(p, (AstNode){
        .type = AST_FOR,
        .start = tok.start,
        .value.first_child = inner,
      });
    case TOK_GOTO:
      p->pos++;
      Token ident = p->tokens[p->pos++];
      assert(ident.type = TOK_IDENT);
      assert(p->tokens[p->pos++].type == TOK_SEMICOLON);
      label = Parser_resolve_label(p, (Str){ &p->source[ident.start], ident.len });
      assert(label);
      return Parser_create_expr(p, (AstNode){
        .type = AST_GOTO,
        .start = tok.start,
        .value.label = label,
      });
    case TOK_CONTINUE:
      p->pos++;
      assert(p->tokens[p->pos++].type == TOK_SEMICOLON);
      return Parser_create_expr(p, (AstNode){
        .type = AST_CONTINUE,
        .start = tok.start,
        .value.first_child = inner,
      });
    case TOK_BREAK:
      p->pos++;
      assert(p->tokens[p->pos++].type == TOK_SEMICOLON);
      return Parser_create_expr(p, (AstNode){
        .type = AST_BREAK,
        .start = tok.start,
        .value.first_child = inner,
      });
    case TOK_RETURN:
      if (p->tokens[++p->pos].type != TOK_SEMICOLON) {
        inner = Parser_parse_expression(p);
        assert(p->tokens[p->pos].type == TOK_SEMICOLON);
      }
      p->pos++;
      return Parser_create_expr(p, (AstNode){
        .type = AST_RETURN,
        .start = tok.start,
        .value.first_child = inner,
      });
    default:
      break;
  }
  uint16_t expr = Parser_parse_expression(p);
  assert(p->tokens[p->pos++].type == TOK_SEMICOLON);
  return expr;
}

