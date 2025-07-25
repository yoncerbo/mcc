#include "ast.h"
#include "common.h"
#include "tokens.h"
#include "parser.h"
#include <assert.h>
#include <stdint.h>
#include <string.h>

uint16_t Parser_push_typedef(Parser *p, Typedef td) {
  assert(p->typedefs_size < MAX_VARIABLES);
  for (int i = 1; i < p->typedefs_size; ++i) {
    if (p->typedefs[i].len != td.len) continue;
    assert(strncmp(&p->source[p->typedefs[i].start], &p->source[td.start], td.len));
  }
  uint16_t index = p->typedefs_size++;
  p->typedefs[index] = td;
  return index;
}

uint16_t Parser_resolve_typedef(Parser *p, uint32_t start, uint16_t len) {
  for (uint16_t i = 0; i < p->typedefs_size; ++i) {
    if (len != p->typedefs[i].len) continue;
    if (!strncmp(&p->source[p->typedefs[i].start], &p->source[start], len)) return i;
  }
  return 0;
}

// note: lable scope is per function
uint16_t Parser_push_label(Parser *p, Str name) {
  assert(p->labels_size < MAX_VARIABLES);
  for (int i = 1; i < p->labels_size; ++i) {
    if (p->labels[i].len != name.len) continue;
    assert(strncmp(p->labels[i].ptr, name.ptr, name.len));
  }
  uint16_t index = p->labels_size++;
  p->labels[index] = name;
  return index;
}

// TODO: per funciton labels
uint16_t Parser_resolve_label(Parser *p, Str name) {
  for (uint16_t i = p->labels_size - 1; i > 0; --i) {
    if (name.len != p->labels[i].len) continue;
    if (!strncmp(p->labels[i].ptr, name.ptr, name.len)) return i;
  }
  return 0;
}

VarId Parser_push_var(Parser *p, Var var) {
  assert(p->var_size < MAX_VARIABLES);
  uint16_t index = p->var_size++;
  // Check for variables in the scope with the same name
  Scope *scope = &p->scopes[p->scope];
  for (int i = scope->start; i < scope->start + scope->len; ++i) {
    if (p->vars[i].name.len != var.name.len) continue;
    assert(strncmp(p->vars[i].name.ptr, var.name.ptr, var.name.len));
  }
  p->vars[index] = var;
  scope->len++;
  return index;
}

uint16_t Parser_resolve_var(Parser *p, Str name) {
  for (int i = p->scope; i >= 0; --i) {
    Scope scope = p->scopes[i];
    for (int i = scope.start + scope.len; i >= scope.start; --i) {
      if (name.len != p->vars[i].name.len) continue;
      if (strncmp(p->vars[i].name.ptr, name.ptr, name.len)) continue;
      p->vars[i].usage++;
      return i;
    }
  }
  return 0;
}

void Parser_push_scope(Parser *p) {
  assert(p->scope++ < MAX_SCOPES);
  p->scopes[p->scope] = (Scope){ p->var_size, 0 };
}

void Parser_pop_scope(Parser *p) {
  assert(p->scope--);
}

uint16_t Parser_create_expr(Parser *p, AstNode expr) {
  assert(p->ast_size < MAX_AST_SIZE);
  uint16_t index = p->ast_size++;
  p->ast_out[index] = expr;
  return index;
}

uint16_t Parser_create_ident(Parser *p, Token source) {
  return Parser_create_expr(p, (AstNode){
    .type = AST_IDENT,
    .start = source.start,
    .value.len = source.len,
  });
}

uint16_t parse(const char *source, const Token *tokens, AstNode *ast_out, Parser *p) {
  *p = (Parser){ 
    .source = source,
    .tokens = tokens,
    .ast_out = ast_out,
    .var_size = 1, // 0 means not found or invalid
    .ast_size = 1, // leave the first empty, to use zero for no children
    .labels_size = 1, // same as above
  };

  assert(tokens[0].type == TOK_INT);
  assert(tokens[1].type == TOK_IDENT);
  assert(!strncmp(&source[tokens[1].start], "main", 4));
  assert(tokens[2].type == TOK_LPAREN);
  assert(tokens[3].type == TOK_VOID);
  assert(tokens[4].type == TOK_RPAREN);
  assert(tokens[5].type == TOK_LBRACE);
  p->pos = 6;

  return Parser_parse_block(p);
}

void print_ast(Parser *p, uint16_t node, int indent_level) {
  Str name;
  while (1) {
    AstNode expr = p->ast_out[node];
    for (int i = 0; i < indent_level * 2; ++i) putchar(' ');
    printf("%s ", AST_TYPE_STR[expr.type]);
    switch (expr.type) {
      case AST_INT:
        printf("%ld\n", expr.value.i64);
        break;
      case AST_DECL:
        putchar(10);
        int i = 0;
        for (; i < (int)(expr.value.decl.var_count); i++) {
          for (int i = 0; i < (indent_level + 1) * 2; ++i) putchar(' ');
          Var var = p->vars[expr.value.decl.var_start + i];
          printf("%s ", STORAGE_TO_STR[var.storage]);
          if (var.flags & FLAG_CONST) printf("const ");
          if (var.flags & FLAG_RESTRICT) printf("restrict ");
          if (var.flags & FLAG_VOLATILE) printf("volatile ");
          printf("%s %.*s, usage=%d\n", DATA_TYPE_TO_STR[var.type],
              var.name.len, var.name.ptr, var.usage);
        }
        if (!expr.value.first_child) break;
        print_ast(p, expr.value.first_child, indent_level + 1);
        break;
      case AST_VAR:
        name = p->vars[expr.value.var].name;
        printf("%.*s\n", name.len, name.ptr);
        break;
      case AST_IDENT:
        printf("%.*s\n", expr.value.len, &p->source[expr.start]);
        break;
      case AST_GOTO:
      case AST_LABEL:
        name = p->labels[expr.value.label];
        printf("%.*s\n", name.len, name.ptr);
        break;
      default:
        putchar(10);
        if (!expr.value.first_child) break;
        print_ast(p, expr.value.first_child, indent_level + 1);
        break;
    }
    if (!expr.next_sibling) break;
    node = expr.next_sibling;
  }
}
