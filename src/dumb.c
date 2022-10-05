#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>

#include "dumb.h"
#include "log.h"
#include "str.h"

dumbfile* dumbfile_init(char* path) {
  FILE* fd = fopen(path, "r");

  if (!fd) {
    ERROR("could not open file %s: %s", path, strerror(errno));
    return 0;
  }

  dumbfile* d = malloc(sizeof *d);
  size_t buflen = 128;
  char* buf = malloc(buflen);
  dumbfile_entry** entry = &d->entry;

  for (size_t n; (n = getline(&buf, &buflen, fd)) != -1;) {
    char* key = strtok(buf, "=");
    char* val = strtok(0, "\0");

    if (key && val) {
      *entry = malloc(sizeof **entry);
      *(*entry) = (dumbfile_entry) { 
        .key = strdup(strtrim(key)),
        .val = strdup(strtrim(val)),
      };
      entry = &(*entry)->next;
    } else {
      WARN("invalid line");
    }
  }

  free(buf);
  return d;
}

void dumbfile_destroy(dumbfile* d) {
  if (!d) {
    return;
  }
  for (dumbfile_entry* entry = d->entry; entry;) {
    dumbfile_entry* next = entry->next;
    free(entry->key);
    free(entry->val);
    free(entry);
    entry = next;
  }
  free(d);
}
