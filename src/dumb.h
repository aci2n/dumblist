#ifndef DUMB_H
#define DUMB_H 1

typedef struct dumbfile_entry dumbfile_entry;
struct dumbfile_entry {
  char* key;
  char* val;
  dumbfile_entry* next;
};

typedef struct dumbfile dumbfile;
struct dumbfile {
  dumbfile_entry* entry;
};

dumbfile* dumbfile_init(char* path);
void dumbfile_destroy(dumbfile* d);

#endif
