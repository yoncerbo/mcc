#include "ast.h"
#include "tokens.h"
#include <assert.h>
#include <stdint.h>

// TODO: reorder the tokens in the definition
AstExprType tok2operation[TOK_IDENT] = {
  // For postfix expression
  [TOK_LSQUARE] = AST_INDEX,
  [TOK_LPAREN] = AST_CALL,
  [TOK_DOT] = AST_DOT,
  [TOK_ARROW] = AST_ARROW,
  [TOK_DPLUS] = AST_POST_INC,
  [TOK_DMINUS] = AST_POST_DEC,

  // For binary operators
  [TOK_STAR] = AST_MUL, [TOK_SLASH] = AST_DIV, [TOK_PERCENT] = AST_MUL,
  [TOK_PLUS] = AST_ADD, [TOK_MINUS] = AST_SUB, [TOK_LSFT] = AST_LSFT,
  [TOK_RSFT] = AST_RSFT, [TOK_LT] = AST_LT, [TOK_LE] = AST_LE,
  [TOK_GT] = AST_GT, [TOK_GE] = AST_GE, [TOK_DEQ] = AST_EQ,
  [TOK_NEQ] = AST_NE, [TOK_AND] = AST_BAND, [TOK_HAT] = AST_BXOR,
  [TOK_OR] = AST_BOR, [TOK_DAND] = AST_LAND, [TOK_DOR] = AST_LOR,

  // For assignments
  [TOK_EQ] = AST_ASS, [TOK_STAR_EQ] = AST_ASS_MUL, [TOK_SLASH_EQ] = AST_ASS_DIV,
  [TOK_PERCENT_EQ] = AST_ASS_MOD, [TOK_PLUS_EQ] = AST_ASS_ADD, [TOK_MINUS_EQ] = AST_ASS_SUB,
  [TOK_LSFT_EQ] = AST_ASS_LSFT, [TOK_RSF_EQ] = AST_ASS_RSFT, [TOK_AND_EQ] = AST_ASS_AND,
  [TOK_HAT_EQ] = AST_ASS_XOR, [TOK_OR_EQ] = AST_ASS_OR,
};

// TODO: reorder the ast types and limite the size
uint8_t op2precedence[AST_COUNT] = {
  [AST_MUL] = 10, [AST_DIV] = 10, [AST_MOD] = 10,
  [AST_ADD] = 9, [AST_SUB] = 9, [AST_LSFT] = 8,
  [AST_RSFT] = 8, [AST_LT] = 7, [AST_LE] = 7,
  [AST_GT] = 7, [AST_GE] = 7, [AST_EQ] = 6,
  [AST_NE] = 6, [AST_BAND] = 5, [AST_BXOR] = 4,
  [AST_BOR] = 3, [AST_LAND] = 2, [AST_LOR] = 1,
};

typedef struct {
  const char *source;
  const Token *tokens;
  AstExpr *ast_out;
  uint16_t pos;
  uint16_t ast_size;
} Parser;

static inline uint16_t Parser_create_expr(Parser *p, AstExpr expr) {
  assert(p->ast_size < MAX_AST_SIZE);
  uint16_t index = p->ast_size++;
  p->ast_out[index] = expr;
  return index;
}

uint16_t Parser_parse_expression(Parser *p);

uint16_t Parser_parse_primary(Parser *p) {
  uint16_t index;
  Token tok = p->tokens[p->pos++];
  switch (tok.type) {
    case TOK_IDENT:
      return Parser_create_expr(p, (AstExpr){
          .type = AST_IDENT,
          .start = tok.start,
          .value.len = tok.len,
      });
    case TOK_DECIMAL:
      int64_t number = 0;
      for (uint16_t i = 0; i < tok.len; ++i) {
        number *= 10;
        number += p->source[tok.start + i] - '0';
      }
      return Parser_create_expr(p, (AstExpr){
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
    AstExprType op = tok2operation[tok.type];
    if (op < AST_INDEX) break;
    if (op < AST_DOT) {
      p->pos++;
      right = Parser_parse_expression(p);
      TokenType tt = op == AST_INDEX ? TOK_RSQUARE : TOK_RPAREN;
      assert(p->tokens[p->pos].type == tt);
      p->ast_out[left].next_sibling = right;
    } else if (op < AST_POST_INC) {
      ident = p->tokens[++p->pos];
      assert(ident.type == TOK_IDENT);
      right = Parser_create_expr(p, (AstExpr){
        .type = AST_IDENT,
        .start = ident.start,
        .value.len = ident.len,
      });
      p->ast_out[left].next_sibling = right;
    }
    left = Parser_create_expr(p, (AstExpr){
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
  AstExprType op = 0;
  // TODO: make another lookup table
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
  }
  if (!op) return Parser_parse_postfix(p);
  p->pos++;
  uint16_t arg =  Parser_parse_unary(p);
  return Parser_create_expr(p, (AstExpr){
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
    AstExprType op = tok2operation[tok.type];
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
    left =  Parser_create_expr(p, (AstExpr){
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
  if (p->tokens[p->pos].type != TOK_QUESTION) return left;
  p->pos++;
  uint16_t then = Parser_parse_expression(p);
  assert(p->tokens[p->pos++].type == TOK_COLON);
  uint16_t els = Parser_parse_conditional(p, Parser_parse_unary(p));

  p->ast_out[cond].next_sibling = then;
  p->ast_out[then].next_sibling = els;
  return Parser_create_expr(p, (AstExpr){
    .type = AST_CONDITIONAL,
    .value.first_child = cond,
    .start = p->ast_out[cond].start,
  });
}

uint16_t Parser_parse_assignment(Parser *p) {
  uint16_t right;
  uint16_t left = Parser_parse_unary(p);
  Token tok = p->tokens[p->pos];
  if (tok.type > TOK_IDENT) return Parser_parse_conditional(p, left);
  AstExprType op = tok2operation[tok.type];
  if (op < AST_ASS) return Parser_parse_conditional(p, left);
  p->pos++;
  right = Parser_parse_assignment(p);
  p->ast_out[left].next_sibling = right;
  return Parser_create_expr(p, (AstExpr){
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

uint16_t parse(const char *source, const Token *tokens, AstExpr *ast_out) {
  Parser p = { source, tokens, ast_out };

  assert(tokens[0].type == TOK_INT);
  assert(tokens[1].type == TOK_IDENT);
  assert(!strncmp(&source[tokens[1].start], "main", 4));
  assert(tokens[2].type == TOK_LPAREN);
  assert(tokens[3].type == TOK_VOID);
  assert(tokens[4].type == TOK_RPAREN);
  assert(tokens[5].type == TOK_LBRACE);
  assert(tokens[6].type == TOK_RETURN);
  p.pos = 7;

  return Parser_parse_expression(&p);
}

void print_ast(const char *source, const AstExpr *ast, uint16_t start, int indent_level) {
  AstExpr expr;
  while (1) {
    AstExpr expr = ast[start];
    for (int i = 0; i < indent_level * 2; ++i) putchar(' ');
    printf("%s: ", AST_TYPE_STR[expr.type]);
    switch (expr.type) {
      case AST_INT:
        printf("%ld\n", expr.value.i64);
        break;
      case AST_IDENT:
        printf("%.*s\n", expr.value.len, &source[expr.start]);
        break;
      default:
        putchar(10);
        print_ast(source, ast, expr.value.first_child, indent_level + 1);
        break;
    }
    if (!expr.next_sibling) break;
    start = expr.next_sibling;
  }
}
