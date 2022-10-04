#ifndef HTTP_H
#define HTTP_H 1

typedef struct httpheader httpheader;
struct httpheader {
  char* key;
  char* val;
  httpheader* next;
};

typedef struct httpreqline httpreqline;
struct httpreqline {
  char* method;
  char* path;
  char* version;
};

typedef struct httpreq httpreq;
struct httpreq {
  httpreqline reqline;
  httpheader* header;
};

httpreq* httpreq_init(int fd);
void httpreq_destroy(httpreq req[static 1]);

#endif
