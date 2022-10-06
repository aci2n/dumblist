#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "str.h"

char* strtrim(char str[static 1]) {
  size_t len = strlen(str);
  size_t start = 0;
  size_t end = len;

  for (; start < len && isspace(str[start]); start++);
  for (; end > 0 && isspace(str[end-1]); end--);

  size_t n = end - start + 1;
  char* copy = malloc(n);

  strncpy(copy, str + start, n);

  return copy;
}

bool strendswith(char str[static 1], char suf[static 1]) {
  str = strrchr(str, '.');
  return str && strcmp(str, suf) == 0;
}

/* p is guaranteed to be pow of 2 */
static size_t closest_pow2(size_t x, size_t p) {
  while (x > p) {
    p <<= 1;
  }
  return p;
}

strbuf* strbuf_init(size_t initial_cap) {
  strbuf* sb = malloc(sizeof *sb);
  size_t cap = closest_pow2(initial_cap, 2);

  *sb = (strbuf) {
    .data = malloc(cap),
    .cap = cap,
    .len = 0,
  };

  return sb;
}

void strbuf_grow(strbuf* sb, size_t n) {
  size_t req_cap = sb->len + n;

  if (req_cap > sb->cap) {
    size_t new_cap = closest_pow2(req_cap, sb->cap);
    sb->data = realloc(sb->data, new_cap);
    sb->cap = new_cap;
  }
}

void strbuf_add(strbuf* sb, char const* s) {
  size_t n = strlen(s);
  strbuf_grow(sb, n + 1);
  strncpy(sb->data + sb->len, s, n + 1);
  sb->len += n;
}

size_t strbuf_len(strbuf* sb) {
  return sb->len;
}

void strbuf_destroy(strbuf* sb) {
  if (sb) {
    free(sb->data);
    free(sb);
  }
}

char* strnum(long num) {
  size_t len = snprintf(0, 0, "%ld", num) + 1;
  char* buf = malloc(len);
  snprintf(buf, len, "%ld", num);
  return buf;
}

char* strbuf_data(strbuf* sb) {
  return sb->data;
}
