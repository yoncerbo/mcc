#include "ast.h"
#include "parser.h"
#include "tokens.h"

uint16_t Parser_parse_primary(Parser *p) {
  uint16_t index;
  Token tok = p->tokens[p->pos++];
  switch (tok.type) {
    case TOK_IDENT:
      VarId var = Parser_resolve_var(p, (Str){ &p->source[tok.start], tok.len });
      assert(var);
      return Parser_create_expr(p, (AstNode){
        .type = AST_VAR,
        .start = tok.start,
        .value.var = var,
      });
    case TOK_DECIMAL:
      int64_t number = 0;
      for (uint16_t i = 0; i < tok.len; ++i) {
        number *= 10;
        number += p->source[tok.start + i] - '0';
      }
      return Parser_create_expr(p, (AstNode){
        .type = AST_INT,
        .start = tok.start,
        .value.i64 = number,
      });
    case TOK_LPAREN:
      index = Parser_parse_expression(p);
      assert(p->tokens[p->pos++].type == TOK_RPAREN);
      return index;
    default:
      assert(0);
  }
}

uint16_t Parser_parse_postfix(Parser *p) {
  Token ident;
  uint16_t right;
  uint16_t left =  Parser_parse_primary(p);
  while (1) {
    Token tok = p->tokens[p->pos];
    AstType op = tok2operation[tok.type];
    if (op < AST_INDEX) break;
    if (op < AST_DOT) {
      p->pos++;
      right = Parser_parse_expression(p);
      TokenType tt = op == AST_INDEX ? TOK_RSQUARE : TOK_RPAREN;
      assert(p->tokens[p->pos].type == tt);
      p->ast_out[left].next_sibling = right;
    } else if (op < AST_POST_INC) {
      ident = p->tokens[++p->pos];
      // TODO: get rid of that ident wrapping, when you get to structs
      assert(ident.type == TOK_IDENT);
      right = Parser_create_ident(p, ident);
      p->ast_out[left].next_sibling = right;
    }
    left = Parser_create_expr(p, (AstNode){
      .type = op,
      .start = p->ast_out[left].start,
      .value.first_child = left,
    });
    p->pos++;
  }
  return left;
}

uint16_t Parser_parse_unary(Parser *p) {
  Token tok = p->tokens[p->pos];
  AstType op = 0;
  // TODO: make another lookup table or something
  switch (tok.type) {
    case TOK_DPLUS:
      op = AST_PRE_INC;
      break;
    case TOK_DMINUS:
      op = AST_PRE_DEC;
      break;
    case TOK_AND:
      op = AST_ADDR;
      break;
    case TOK_STAR:
      op = AST_DEREF;
      break;
    case TOK_PLUS:
      op = AST_PLUS;
      break;
    case TOK_MINUS:
      op = AST_MINUS;
      break;
    case TOK_TILDA:
      op = AST_NEG;
      break;
    case TOK_NOT:
      op = AST_NOT;
      break;
    case TOK_SIZEOF:
      op = AST_SIZEOF;
      break;
    default:
      return Parser_parse_postfix(p);
  }
  p->pos++;
  uint16_t arg =  Parser_parse_unary(p);
  return Parser_create_expr(p, (AstNode){
    .type = op,
    .start = tok.start,
    .value.first_child = arg,
  });
}

uint16_t Parser_parse_binary(Parser *p, uint8_t precedence, uint16_t left) {
  uint16_t right;
  // uint16_t left_last_child = 0;
  while (1) {
    Token tok = p->tokens[p->pos];
    if (tok.type > TOK_IDENT) break;
    AstType op = tok2operation[tok.type];
    if (!op) break;
    uint8_t new_precedence = op2precedence[op];
    if (new_precedence < precedence) break;
    p->pos++;
    right = Parser_parse_binary(p, new_precedence, Parser_parse_unary(p));
    // Fow now don't combine
    // if (p->ast_out[left].type == op) {
    //   p->ast_out[left_last_child].next_sibling = right;
    //   break;
    // }
    // left_last_child = right;
    p->ast_out[left].next_sibling = right;
    left =  Parser_create_expr(p, (AstNode){
      .type = op,
      .start = p->ast_out[left].start,
      .value.first_child = left,
    });
  }
  return left;
}

// note: the ? and : work like parens
uint16_t Parser_parse_conditional(Parser *p, uint16_t left) {
  uint16_t cond = Parser_parse_binary(p, 0, left);
  if (p->tokens[p->pos].type != TOK_QUESTION) return cond;
  p->pos++;
  uint16_t then = Parser_parse_expression(p);
  assert(p->tokens[p->pos++].type == TOK_COLON);
  uint16_t els = Parser_parse_conditional(p, Parser_parse_unary(p));

  p->ast_out[cond].next_sibling = then;
  p->ast_out[then].next_sibling = els;
  return Parser_create_expr(p, (AstNode){
    .type = AST_CONDITIONAL,
    .value.first_child = cond,
    .start = p->ast_out[cond].start,
  });
}

// TODO: combine assignments of the same type
uint16_t Parser_parse_assignment(Parser *p) {
  uint16_t right;
  uint16_t left = Parser_parse_unary(p);
  Token tok = p->tokens[p->pos];
  if (tok.type > TOK_IDENT) return Parser_parse_conditional(p, left);
  AstType op = tok2operation[tok.type];
  if (op < AST_ASS) return Parser_parse_conditional(p, left);
  p->pos++;
  right = Parser_parse_assignment(p);
  p->ast_out[left].next_sibling = right;
  return Parser_create_expr(p, (AstNode){
    .type = op,
    .value.first_child = left,
    .start = p->ast_out[left].start,
  });
}

uint16_t Parser_parse_expression(Parser *p) {
  uint16_t first = Parser_parse_assignment(p);
  uint16_t last = first;
  while (p->tokens[p->pos].type == TOK_COMMA) {
    p->pos++;
    uint16_t next = Parser_parse_assignment(p);
    p->ast_out[last].next_sibling = next;
    last = next;
  }
  return first;
}

