#include "ast.h"
#include "common.h"
#include "parser.h"
#include "tokens.h"
#include <assert.h>
#include <stdint.h>
#include <stdbool.h>

typedef enum {
  SIGN_NONE,
  SIGN_SIGNED,
  SIGN_UNSIGNED,
} SignType;

typedef enum {
  SIZE_NONE,
  SIZE_SHORT,
  SIZE_LONG,
  SIZE_LONG_LONG,
} SizeType;

typedef struct {
  DataType type;
  StorageType storage;
  VarFlags flags;
} DeclSpecifier;

DeclSpecifier Parser_parse_declaration_specifier(Parser *p) {
  Token tok;
  SizeType size = 0;
  SignType sign = 0;
  VarFlags flags = 0;
  VarFlags typedef_flags = 0;
  DataType dt = DATA_NONE;
  StorageType storage = STORAGE_NONE;

  while(1) {
    tok = p->tokens[p->pos];
    p->pos++;
    // TODO: reorder the tokens, then do compare and addition
    switch (tok.type) {
      // Storage class
      case TOK_TYPEDEF:
        assert(storage == STORAGE_NONE);
        storage = STORAGE_TYPEDEF;
        break;
      case TOK_EXTERN:
        assert(storage == STORAGE_NONE); // TODO: turn those into errors or warnings
        storage = STORAGE_EXTERN;
        break;
      case TOK_STATIC:
        assert(storage == STORAGE_NONE);
        storage = STORAGE_STATIC;
        break;
      case TOK_AUTO:
        assert(storage == STORAGE_NONE);
        storage = STORAGE_AUTO;
        break;
      case TOK_REGISTER:
        assert(storage == STORAGE_NONE);
        storage = STORAGE_REGISTER;
        break;

      // Type qualifier
      case TOK_CONST:
        assert(flags ^ FLAG_CONST);
        flags |= FLAG_CONST;
        break;
      case TOK_RESTRICT:
        assert(flags ^ FLAG_RESTRICT);
        flags |= FLAG_RESTRICT;
        break;
      case TOK_VOLATILE:
        assert(flags ^ FLAG_VOLATILE);
        flags |= FLAG_VOLATILE;
        break;
      // Function specifier
      case TOK_INLINE:
        assert(flags ^ FLAG_INLINE);
        flags |= FLAG_INLINE;
        break;

      // Base type specifier
      case TOK_VOID:
        assert(dt == DATA_NONE);
        dt = DATA_VOID;
        break;
      case TOK_CHAR:
        assert(dt == DATA_NONE);
        dt = DATA_CHAR;
        break;
      case TOK_INT:
        assert(dt == DATA_NONE);
        dt = DATA_INT;
        break;
      case TOK_FLOAT:
        assert(dt == DATA_NONE);
        dt = DATA_FLOAT;
        break;
      case TOK_DOUBLE:
        assert(dt == DATA_NONE);
        dt = DATA_DOUBLE;
        break;
      case TOK_BOOL:
        assert(dt == DATA_NONE);
        dt = DATA_BOOL;
        break;
      case TOK_COMPLEX:
        assert(dt == DATA_NONE);
        dt = DATA_COMPLEX;
        break;
      // TODO: struct, union, enum, typedef name

      case TOK_SHORT:
        assert(size == SIZE_NONE);
        size = SIZE_SHORT;
        break;
      case TOK_LONG:
        if (size == SIZE_NONE) size = SIZE_LONG;
        else if (size == SIZE_LONG) size = SIZE_LONG_LONG;
        else assert(0);
        break;
      case TOK_SIGNED:
        assert(sign == SIGN_NONE);
        sign = SIGN_SIGNED;
        break;
      case TOK_UNSIGNED:
        assert(sign == SIGN_NONE);
        sign = SIGN_UNSIGNED;
        break;

      // Typedef
      case TOK_IDENT:
        TokenType next = p->tokens[p->pos].type;
        if (next < DECL_SPEC_START && next != TOK_IDENT) {
          p->pos--;
          goto while_end;
        }
        assert(dt == DATA_NONE);
        uint16_t td = Parser_resolve_typedef(p, tok.start, tok.len);
        typedef_flags |= p->typedefs[td].flags;
        dt = p->typedefs[td].type;
        if (next == TOK_IDENT) goto while_end;
        else break;
      default:
        p->pos--;
        goto while_end;
    }
  }
while_end:
  if (storage == STORAGE_NONE) storage = STORAGE_AUTO;
  if (dt == DATA_NONE) dt = DATA_INT;
  flags |= typedef_flags;

  // https://en.wikipedia.org/wiki/C_data_types
  // TODO: float or double _Complex
  // Check the type specifiers and modify the base type
  switch (dt) {
    case DATA_INT:
      dt = DATA_INT + size * 2;
      dt += sign == SIGN_UNSIGNED;
      break;
    case DATA_CHAR:
      assert(size == SIZE_NONE);
      if (sign == SIGN_UNSIGNED) dt = DATA_UCHAR;
      break;
    case DATA_DOUBLE:
      if (size == SIZE_LONG) dt = DATA_LONG_DOUBLE;
      else assert(size == SIZE_NONE);
      assert(sign == SIGN_NONE);
      break;
    default:
      assert(size == SIZE_NONE);
      assert(sign == SIGN_NONE);
  }

  return (DeclSpecifier){
    .type = dt,
    .storage = storage,
    .flags = flags,
  };
}

uint16_t Parser_parse_declaration(Parser *p) {
  Token tok = p->tokens[p->pos];
  TokenType next = p->tokens[p->pos].type;
  // not a specifier and not a symbol, or
  // symbol, but next is not a symbol or specifier
  bool cond = tok.type != TOK_IDENT && tok.type < DECL_SPEC_START;
  cond = cond || (tok.type == TOK_IDENT && next < DECL_SPEC_START && next != TOK_IDENT);
  if (cond) return Parser_parse_statement(p);

  DeclSpecifier spec = Parser_parse_declaration_specifier(p);

  if (spec.storage == STORAGE_TYPEDEF) {
    do {
      Token ident = p->tokens[p->pos++];
      assert(ident.type == TOK_IDENT);
      Typedef td = { ident.start, ident.len, spec.type, spec.flags };
      Parser_push_typedef(p, td);
    } while (p->tokens[p->pos++].type == TOK_COMMA);
    assert(p->tokens[p->pos - 1].type == TOK_SEMICOLON);
    return 0;
    // Parser_parse_declaration(p);
  }

  uint16_t first = 0;
  uint16_t last = 0;
  VarId var_start = 0;
  uint16_t var_count = 0;
  do {
    Token ident = p->tokens[p->pos++];
    assert(ident.type == TOK_IDENT);
    Str name = (Str){ &p->source[ident.start], ident.len };
    uint16_t value = 0;
    if(p->tokens[p->pos].type == TOK_EQ) {
      p->pos++;
      value = Parser_parse_assignment(p);
    } else {
      value = Parser_create_expr(p, (AstNode){
        .type = AST_EMPTY,
      });
    }
    VarId var = Parser_push_var(p, (Var){
      .name = name,
      .usage = 0,
      .storage = spec.storage,
      .type = spec.type,
      .flags = spec.flags,
    });
    if (first) {
      p->ast_out[last].next_sibling = value;
      last = value; 
    } else {
      first = value;
      last = value;
      var_start = var;
    }
    var_count++;
  } while (p->tokens[p->pos++].type == TOK_COMMA);
  assert(p->tokens[p->pos - 1].type == TOK_SEMICOLON);
  return Parser_create_expr(p, (AstNode){
    .type = AST_DECL,
    .start = tok.start,
    .value.decl.first_child = first,
    .value.decl.var_start = var_start,
    .value.decl.var_count = var_count,
  });
}

