#ifndef INCLUDE_COMMON
#define INCLUDE_COMMON

#include "bits/floatn-common.h"
#include <stdint.h>

typedef struct {
  const char *ptr;
  uint32_t len;
} Str;

typedef struct {
  uint32_t start;
  uint32_t len;
} Span;

#define STRINGIFY_INNER(x) #x
#define STRINGIFY(x) STRINGIFY_INNER(x) 
#define CSTR_LEN(str) (sizeof(str) - 1)
#define STR(str) ((Str){ (str), CSTR_LEN(str) })

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define LIMIT_UP(a, b) MIN(a, b)
#define LIMIT_DOWN(a, b) MAX(a, b)

#define DEBUGD(var) \
  printf(STRINGIFY(var) "=%d\n", var)
#define DEBUGC(var) \
  printf(STRINGIFY(var) "='%c'\n", var)
#define DEBUGS(var) \
  printf(STRINGIFY(var) "=\"%s\"\n", var)
#define DEBUGX(var) \
  printf(STRINGIFY(var) "=0x%x\n", var)

#define LOG(fmt, ...) \
  printf("[LOG] %s " __FILE__ ":" STRINGIFY(__LINE__) " " fmt, __func__, ##__VA_ARGS__)

#endif

