#ifndef INCLUDE_PARSER
#define INCLUDE_PARSER

#include "common.h"
#include "ast.h"
#include <stdint.h>

#define MAX_AST_SIZE 512
#define MAX_VARIABLES 256
#define MAX_SCOPES 64
#define MAX_LABELS 64
#define MAX_TYPEDEFS 128
#define MAX_FIELDS 256
#define FIELD_BUFFER_SIZE 64
#define MAX_STRUCTS 64

#define STRUCT_NOT_FOUND UINT16_MAX

typedef uint16_t AstId;
typedef uint16_t VarId;
typedef uint16_t LabelId;
typedef uint16_t TypedefId;
typedef uint16_t StructId;

typedef struct {
  VarId start;
  uint16_t len;
} Scope;

typedef enum {
  FLAG_CONST = 1 << 0,
  FLAG_RESTRICT = 1 << 1,
  FLAG_VOLATILE = 1 << 2,
  FLAG_INLINE = 1 << 3, // only functions
} VarFlags;

typedef struct {
  // TODO: change into start and len
  Str name;
  uint16_t usage;
  uint16_t struct_index;
  StorageType storage;
  DataType type;
  VarFlags flags;
} Var;

typedef enum {
  // first 2 bits
  STRUCT_STRUCT,
  STRUCT_UNION,
  STRUCT_ENUM,

  STRUCT_UNINIT = 1 << 2,
} StructType;

typedef struct {
  uint32_t start;
  uint16_t len; // if zero, then anonymous
  uint16_t fields_start;
  uint16_t fields_len; // zero fields is valid
  StructType type;
} Struct;

typedef struct {
  uint32_t start;
  uint16_t len;
  uint16_t struct_index;
  DataType type;
  VarFlags flags;
} Field;

typedef struct {
  uint32_t start;
  uint16_t len;
  uint16_t struct_index;
  DataType type;
  VarFlags flags;
} Typedef;

typedef struct {
  // note: We need a separate buffer, because
  // struct or union definitons can nest, but we
  // need the fields to be contigious. Not to
  // waste space on unnecessary indices,
  // like we do with ast, and for the field lookup
  // to be faster. As apposed to ast, the number
  // of fields should be much smalle, therefore
  // there should be no problem with copying,
  // After we collect all the fields in the buffer,
  // we're gonna copy them to the fields array.
  Field field_buffer[FIELD_BUFFER_SIZE];
  Field fields[MAX_FIELDS];
  // TODO: move the name into a separate lookup array,
  // as not all structs are gonna have name and it's
  // justa a waste of space, also we're gonna
  // use it for pointers and arrays later
  Struct structs[MAX_STRUCTS];
  Typedef typedefs[MAX_TYPEDEFS];
  Str labels[MAX_LABELS];
  Var vars[MAX_VARIABLES];
  Scope scopes[MAX_SCOPES];
  const char *source;
  const Token *tokens;
  AstNode *ast_out;
  uint16_t pos;
  uint16_t ast_size;
  uint16_t var_size;
  uint16_t labels_size;
  uint16_t typedefs_size;
  uint16_t fields_size;
  uint16_t field_bufer_size;
  uint16_t structs_size;
  uint8_t scope;
} Parser;

AstId parse(const char *source, const Token *tokens, AstNode *ast_out, Parser *p);
void print_ast(Parser *p, uint16_t node, int indent_level);

AstId Parser_parse_expression(Parser *p);
AstId Parser_parse_assignment(Parser *p);
AstId Parser_parse_unary(Parser *p);
AstId Parser_parse_conditional(Parser *p, uint16_t left);
AstId Parser_parse_declaration(Parser *p);
AstId Parser_parse_statement(Parser *p);
AstId Parser_parse_block(Parser *p);
AstId Parser_create_expr(Parser *p, AstNode expr);
AstId Parser_create_ident(Parser *p, Token source);

LabelId Parser_push_label(Parser *p, Str name);
LabelId Parser_resolve_label(Parser *p, Str name);
VarId Parser_push_var(Parser *p, Var var);
VarId Parser_resolve_var(Parser *p, Str name);
void Parser_push_scope(Parser *p);
void Parser_pop_scope(Parser *p);
TypedefId Parser_push_typedef(Parser *p, Typedef td);
TypedefId Parser_resolve_typedef(Parser *p, uint32_t start, uint16_t len);
StructId Parser_push_struct(Parser *p, Struct s);
StructId Parser_resolve_struct(Parser *p, uint32_t start, uint16_t len);

#endif
