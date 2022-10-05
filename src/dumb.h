#ifndef DUMB_H
#define DUMB_H 1

#define DUMBFILE_EXTENSION ".dumb"

typedef struct dumbfile_entry dumbfile_entry;
struct dumbfile_entry {
  char* key;
  char* val;
  dumbfile_entry* next;
};

typedef struct dumbfile dumbfile;
struct dumbfile {
  char* title;
  dumbfile_entry* entry;
  dumbfile* next;
};

dumbfile* dumbfile_init(char* path);
void dumbfile_destroy(dumbfile* d);
dumbfile* dumbfile_list(char* path);

#endif
