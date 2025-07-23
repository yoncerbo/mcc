#include "ast.h"
#include "tokens.h"
#include <assert.h>
#include <stdint.h>

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
  uint16_t elem =  Parser_parse_primary(p);
  Token tok = p->tokens[p->pos++];
  switch (tok.type) {
    case TOK_DPLUS:
      return Parser_create_expr(p, (AstExpr){
        .type = AST_POST_INC,
        .start = p->ast_out[elem].start,
        .value.first_child = elem,
      });
    case TOK_DMINUS:
      return Parser_create_expr(p, (AstExpr){
        .type = AST_POST_DEC,
        .start = p->ast_out[elem].start,
        .value.first_child = elem,
      });
    default:
      p->pos--;
      return elem;
  }
}

uint16_t Parser_parse_unary(Parser *p) {
  uint16_t elem;
  Token tok = p->tokens[p->pos++];
  switch (tok.type) {
    case TOK_DPLUS:
      elem =  Parser_parse_postfix(p);
      return Parser_create_expr(p, (AstExpr){
        .type = AST_PRE_INC,
        .start = p->ast_out[elem].start,
        .value.first_child = elem,
      });
    case TOK_DMINUS:
      elem =  Parser_parse_postfix(p);
      return Parser_create_expr(p, (AstExpr){
        .type = AST_PRE_DEC,
        .start = p->ast_out[elem].start,
        .value.first_child = elem,
      });
    default:
      p->pos--;
      return Parser_parse_postfix(p);
  }
}

uint16_t Parser_parse_multiplicative(Parser *p) {
  uint16_t right;
  uint16_t left = Parser_parse_unary(p);
  uint16_t left_last_child = 0;
  while (1) {
    AstExprType type;
    Token tok = p->tokens[p->pos];
    switch (tok.type) {
      case TOK_STAR:
        type = AST_MUL;
        break;
      case TOK_SLASH:
        type = AST_DIV;
        break;
      case TOK_PERCENT:
        type = AST_MOD;
        break;
      default:
        return left;
    }
    p->pos++;
    right = Parser_parse_unary(p);
    if (p->ast_out[left].type == type) {
      p->ast_out[left_last_child].next_sibling = right;
      break;
    }
    left_last_child = right;
    p->ast_out[left].next_sibling = right;
    left =  Parser_create_expr(p, (AstExpr){
      .type = type,
      .start = p->ast_out[left].start,
      .value.first_child = left,
    });
  }
  return left;
}

uint16_t Parser_parse_additive(Parser *p) {
  uint16_t right;
  uint16_t left = Parser_parse_multiplicative(p);
  uint16_t left_last_child = 0;
  while (1) {
    AstExprType type;
    Token tok = p->tokens[p->pos];
    switch (tok.type) {
      case TOK_PLUS:
        type = AST_ADD;
        break;
      case TOK_MINUS:
        type = AST_SUB;
        break;
      default:
        return left;
    }
    p->pos++;
    right = Parser_parse_multiplicative(p);
    if (p->ast_out[left].type == type) {
      p->ast_out[left_last_child].next_sibling = right;
      break;
    }
    left_last_child = right;
    p->ast_out[left].next_sibling = right;
    left =  Parser_create_expr(p, (AstExpr){
      .type = type,
      .start = p->ast_out[left].start,
      .value.first_child = left,
    });
  }
  return left;
}
uint16_t Parser_parse_expression(Parser *p) {
  return Parser_parse_additive(p);
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
