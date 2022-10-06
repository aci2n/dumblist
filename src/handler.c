#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <errno.h>
#include <arpa/inet.h>

#include "handler.h"
#include "http.h"
#include "log.h"
#include "dumb.h"
#include "str.h"

static void build_content(strbuf* sb, dumbfile* list) {
  size_t count = 0;

  strbuf_add(sb, "<!DOCTYPE HTML><html><head></head><body><h1>Listing</h1><main>");
  for (dumbfile* df = list; df; df = df->next) {
    strbuf_add(sb, "<article>");
    strbuf_printf(sb, "<h3>%lu</h3>", count);
    strbuf_add(sb, "<dl>");
    for (dumbfile_entry *e = df->entry; e; e = e->next) {
      strbuf_printf(sb, "<dt>%s</dt>", e->key);
      strbuf_printf(sb, "<dd>%s</dd>", e->val);
    }
    strbuf_add(sb, "</dl></article>");
  }
  strbuf_add(sb, "</main></body></html>");
}

#define HTTP_NEWLINE "\r\n"

static void build_headers(strbuf* sb, size_t content_len) {
  strbuf_add(sb, "HTTP/1.1 200 OK" HTTP_NEWLINE);
  strbuf_add(sb, "content-type: text/html; charset=us-acsii" HTTP_NEWLINE);
  strbuf_printf(sb, "content-length: %lu" HTTP_NEWLINE, content_len);
  strbuf_add(sb, HTTP_NEWLINE);
}

static void send_response(httpreq* req, dumbfile* list) {
  strbuf* headers = strbuf_init(128);
  strbuf* content = strbuf_init(256);

  build_content(content, list);
  DEBUG("response body len: %lu", strbuf_len(content));

  build_headers(headers, strbuf_len(content));
  DEBUG("headers len: %lu", strbuf_len(headers));

  httpreq_send(req, strbuf_len(headers), strbuf_data(headers));
  httpreq_send(req, strbuf_len(content), strbuf_data(content));
}

/* we only support GETs :D */
int handle_client_req(int fd, char* datadir) {
  httpreq* req = httpreq_init(fd);

  if (!req) {
    return -1;
  }

  INFO("HTTP request: [%s,%s,%s]", req->reqline.version, req->reqline.method, req->reqline.path);
#if LOG_LEVEL >= DEBUG
  for (httpheader* header = req->header; header; header = header->next) {
    DEBUG("%s=%s", header->key, header->val);
  }
#endif

  dumbfile* file = dumbfile_list(datadir);

  if (!file) {
    ERROR("no files found in datadir %s", datadir);
    goto done;
  }

  send_response(req, file);

done:
  dumbfile_destroy(file);
  httpreq_destroy(req);
  return 0;
}
