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

uint16_t Parser_parse_expression(Parser *p) {
  Token tok = p->tokens[p->pos];
  switch (tok.type) {
    case TOK_DECIMAL: {
      int64_t number = 0;
      for (uint16_t i = 0; i < tok.len; ++i) {
        number *= 10;
        number += p->source[tok.start + i] - '0';
      }
      assert(p->ast_size < MAX_AST_SIZE);
      uint16_t index = p->ast_size++;
      p->ast_out[index] = (AstExpr){
        .type = AST_INT,
        .start = tok.start,
        .value.i64 = number,
      };
      p->pos++;
      return index;
    };
    case TOK_NONE:
    default:
      return 0;
  }
}

void parse(const char *source, const Token *tokens, AstExpr *ast_out) {
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

  Parser_parse_expression(&p);
}

void print_ast(const AstExpr *ast) {
  while (ast->type) {
    printf("% 3d:%02d %s: ", ast->start, ast->len, AST_TYPE_STR[ast->type]);
    switch (ast->type) {
      case AST_INT:
        printf("%ld\n", ast->value.i64);
        break;
    }
    ast++;
  }
}
