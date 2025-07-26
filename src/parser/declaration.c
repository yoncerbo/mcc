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
  uint16_t struct_index;
} DeclSpecifier;

DeclSpecifier Parser_parse_declaration_specifier(Parser *p);

void Parser_parse_struct_fileds(Parser *p, uint16_t struct_index) {
  assert(struct_index != STRUCT_NOT_FOUND);
  uint16_t start = p->field_bufer_size;
  uint16_t len = 0;
  while (p->tokens[p->pos].type != TOK_RBRACE) {
    DeclSpecifier spec = Parser_parse_declaration_specifier(p);
    // Disallowed in struct fields
    assert(spec.storage == STORAGE_NONE);
    assert(spec.flags ^ FLAG_INLINE);
    do {
      Token ident = p->tokens[p->pos++];
      assert(ident.type == TOK_IDENT);
      assert(p->field_bufer_size < FIELD_BUFFER_SIZE);
      uint16_t index = p->field_bufer_size++;
      p->field_buffer[index] = (Field){
        .start = ident.start,
        .len = ident.len,
        .type = spec.type,
        .struct_index = spec.struct_index,
        .flags = spec.flags,
      };
      len++;
    } while (p->tokens[p->pos++].type == TOK_COMMA);
    assert(p->tokens[p->pos - 1].type == TOK_SEMICOLON);
  }
  p->pos++;
  assert(p->fields_size + len < MAX_FIELDS);
  memcpy(&p->fields[p->fields_size], &p->field_buffer[start], len * sizeof(Field));
  p->structs[struct_index].fields_start = p->fields_size += len;
  p->structs[struct_index].fields_len = len;
}


DeclSpecifier Parser_parse_declaration_specifier(Parser *p) {
  Token tok;
  SizeType size = 0;
  SignType sign = 0;
  VarFlags flags = 0;
  VarFlags typedef_flags = 0;
  DataType dt = DATA_NONE;
  StorageType storage = STORAGE_NONE;
  uint16_t struct_index = 0;

  while(1) {
    // TODO: I'm kinda dissatisfied with how this whole
    // function works, but I'm just gonna leave it
    // at least it takes less space than before
    tok = p->tokens[p->pos];
    if (tok.type == TOK_IDENT) {
      TokenType next = p->tokens[p->pos + 1].type;
      if (next < DECL_SPEC_START && next != TOK_IDENT) break;
      p->pos++;
      assert(dt == DATA_NONE);
      uint16_t td = Parser_resolve_typedef(p, tok.start, tok.len);
      typedef_flags |= p->typedefs[td].flags;
      dt = p->typedefs[td].type;
      struct_index = p->typedefs[td].struct_index;
      if (next == TOK_IDENT) break;
      else continue;
    }
    if (tok.type < DECL_SPEC_START) break;
    p->pos++;
    if (tok.type <= TOK_TYPEDEF) {
      // TODO: error if struct field definition
      assert(storage == STORAGE_NONE);
      storage = tok.type - TOK_EXTERN;
      continue;
    } else if (tok.type <= TOK_INLINE) {
      VarFlags flag = 1 << (tok.type - TOK_CONST);
      assert(flags ^ flag);
      flags |= flag;
      continue;
    } else if (tok.type <= TOK_INT) {
      assert(dt == DATA_NONE);
      dt = tok.type - TOK_INLINE; // one lower, because of DATA_NONE
      continue;
    } else if (tok.type <= TOK_UNION) {
      assert(dt == DATA_NONE);
      dt = DATA_STRUCT + (tok.type - TOK_STRUCT);
      StructType st = tok.type - TOK_STRUCT;
      Token ident = p->tokens[p->pos++];
      // Create anonymous
      if (ident.type == TOK_LBRACE) {
        p->pos++;
        struct_index = Parser_push_struct(p, (Struct){ .type = st });
        Parser_parse_struct_fileds(p, struct_index);
        continue;
      }
      assert(ident.type = TOK_IDENT);
      struct_index = Parser_resolve_struct(p, ident.start, ident.len);
      // Initialize uninitialized
      if (p->tokens[p->pos].type == TOK_LBRACE) {
        p->pos++;
        if (struct_index == STRUCT_NOT_FOUND) {
          struct_index = Parser_push_struct(p, (Struct){
            .start = ident.start,
            .len = ident.len,
            .type = st,
          });
        } else assert(p->structs[struct_index].type == st);
        Parser_parse_struct_fileds(p, struct_index);
        continue;
      }
      // Create uninitialized
      if (struct_index == STRUCT_NOT_FOUND) {
        struct_index = Parser_push_struct(p, (Struct){
          .start = ident.start,
          .len = ident.len,
          .type = st | STRUCT_UNINIT,
        });
        continue;
      }
      // Check if it's adequate type, if initialized
      assert((p->structs[struct_index].type & 3) == st);
      continue;
    }
    switch (tok.type) {
      case TOK_SHORT:
        assert(size == SIZE_NONE);
        size = SIZE_SHORT;
        break;
      case TOK_SIGNED:
        assert(sign == SIGN_NONE);
        sign = SIGN_SIGNED;
        break;
      case TOK_UNSIGNED:
        assert(sign == SIGN_NONE);
        sign = SIGN_UNSIGNED;
        break;
      case TOK_LONG:
        if (size == SIZE_NONE) size = SIZE_LONG;
        else if (size == SIZE_LONG) size = SIZE_LONG_LONG;
        else assert(0);
        break;
      default:
        assert(0);
    }
  }
  if (dt == DATA_NONE) dt = DATA_INT;
  flags |= typedef_flags;

  // https://en.wikipedia.org/wiki/C_data_types
  // TODO: float or double _Complex
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
    .struct_index = struct_index,
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
  if (spec.storage == STORAGE_NONE) spec.storage = STORAGE_AUTO;

  if (spec.storage == STORAGE_TYPEDEF) {
    do {
      Token ident = p->tokens[p->pos++];
      assert(ident.type == TOK_IDENT);
      Typedef td = { ident.start, ident.len, spec.struct_index, spec.type, spec.flags };
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
    if (!first && ident.type == TOK_SEMICOLON) return 0;
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

