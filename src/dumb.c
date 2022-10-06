#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <dirent.h>
#include <errno.h>
#include <sys/types.h>

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
    TRACE("reading dumbfile line: %s", buf);

    char* key = strtok(buf, "=");
    char* val = strtok(0, "\0");

    if (key && val) {
      dumbfile_entry* e = malloc(sizeof *e);

      *e = (dumbfile_entry) { 
        .key = strtrim(key),
        .val = strtrim(val),
      };
      *entry = e;
      entry = &e->next;

      TRACE("dumbfile entry: [%s=%s]", e->key, e->val);
    } else {
      WARN("invalid line");
    }
  }

  free(buf);
  return d;
}

void dumbfile_destroy(dumbfile* df) {
  if (!df) {
    return;
  }
  for (dumbfile* curr = df; curr;) {
    for (dumbfile_entry* entry = curr->entry; entry;) {
      dumbfile_entry* next = entry->next;
      free(entry->key);
      free(entry->val);
      free(entry);
      entry = next;
    }
    dumbfile* next = df->next;
    free(curr);
    curr = next;
  }
}

dumbfile* dumbfile_list(char* path) {
  DIR* fd = 0;
  dumbfile* list = 0;
  dumbfile** next = &list;

  fd = opendir(path);

  if (!fd) {
    ERROR("opendir: %s", strerror(errno));
    goto done;
  }

  while (true) {
    errno = 0;
    struct dirent* de = readdir(fd);

    if (!de) {
      if (errno) {
        ERROR("readdir: %s", strerror(errno));
      } else{
        TRACE("finished reading dir %s", path);
      }
      break;
    }

    TRACE("found file: %s", de->d_name);

    if (strendswith(de->d_name, DUMBFILE_EXTENSION)) {
      char fullpath[PATH_MAX];
      dumbfile* f = 0;

      snprintf(fullpath, sizeof fullpath, "%s/%s", path, de->d_name);
      f = dumbfile_init(fullpath);

      if (f) {
        *next = f;
        next = &f->next;
      }
    }
  }

done:
  closedir(fd);
  return list;
}
