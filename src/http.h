#ifndef HTTP_H
#define HTTP_H 1

#include <stdlib.h>
#include <sys/sendfile.h>

#include "str.h"

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
  int fd;
  httpreqline reqline;
  httpheader* header;
};

typedef enum http_content_type http_content_type;
enum http_content_type {
  CT_ASCII,
  CT_JPG,
  http_content_type_count,
};

typedef struct httpresp httpresp;
struct httpresp {
  int fd;
  unsigned sc;
  http_content_type content_type;
  size_t content_length;
  strbuf* body;
  char* file_path;
};

httpreq* httpreq_create(int fd);
void httpreq_destroy(httpreq* req);

httpresp* httpresp_create(int fd);
int httpresp_send(httpresp* resp);
void httpresp_destroy(httpresp* resp);

#endif
