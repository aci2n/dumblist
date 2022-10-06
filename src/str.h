#ifndef STR_H
#define STR_H 1

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

#include "log.h"

char* strtrim(char str[static 1]);
bool strendswith(char str[static 1], char suf[static 1]);
char* strnum(long num);

typedef struct strbuf strbuf;
struct strbuf {
  char* data;
  size_t cap;
  size_t len;
};

strbuf* strbuf_init(size_t initial_cap); 
void strbuf_add(strbuf* sb, char const* s);
size_t strbuf_len(strbuf* sb); 
void strbuf_destroy(strbuf* sb);
char* strbuf_data(strbuf* sb);
void strbuf_grow(strbuf* sb, size_t n);

#define strbuf_printf(SB, FORMAT, ...) \
  do { \
    size_t n = snprintf(0, 0, "" FORMAT "", __VA_ARGS__); \
    TRACE("strbuf printf: [format=%s,n=%lu]", "" FORMAT "", n); \
    strbuf_grow(SB, n + 1); \
    snprintf(SB->data + SB->len, n + 1, "" FORMAT "", __VA_ARGS__); \
    SB->len += n; \
  } while (false)

#endif
