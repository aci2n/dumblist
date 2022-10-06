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

#define HTTP_NEWLINE "\r\n"

static strbuf* build_listing(dumbfile* list) {
  strbuf* sb = strbuf_init(512);

  strbuf_add(sb, "<!DOCTYPE HTML><html><head></head><body><header><h1>Listing</h1></header><main>");
  for (dumbfile* df = list; df; df = df->next) {
    strbuf_add(sb, "<article>");
    for (dumbfile_entry *e = df->entry; e; e = e->next) {
      if (strcasecmp(e->key, "title")) {
        strbuf_printf(sb, "<h3>%s</h3>", e->val);
      } else if (strcasecmp(e->key, "image")) {
        strbuf_printf(sb, "<img src=\"%s\">", e->val);
      } else {
        strbuf_printf(sb, "<dl><dt>%s</dt><dd>%s</dd></dl>", e->key, e->val);
      }
    }
    strbuf_add(sb, "</article>");
  }
  strbuf_add(sb, "</main></body></html>");

  TRACE("listing len: %lu", strbuf_len(sb));

  return sb;
}

static strbuf* build_headers(unsigned sc, size_t content_len) {
  strbuf* sb = strbuf_init(128);

  strbuf_printf(sb, "HTTP/1.1 %u" HTTP_NEWLINE, sc);
  strbuf_add(sb, "content-type: text/html; charset=us-acsii" HTTP_NEWLINE);
  strbuf_printf(sb, "content-length: %lu" HTTP_NEWLINE, content_len);
  strbuf_add(sb, HTTP_NEWLINE);

  TRACE("response headers: %s", strbuf_data(sb));

  return sb;
}

static int send_response(httpreq* req, int sc, strbuf* body) {
  strbuf* headers = build_headers(sc, body ? strbuf_len(body) : 0);
  int ret = 0;

  if (httpreq_send(req, strbuf_len(headers), strbuf_data(headers)) == -1) {
    ret = -1;
    goto done;
  }

  if (body && httpreq_send(req, strbuf_len(body), strbuf_data(body)) == -1) {
    ret = -1;
    goto done;
  }

done:
  strbuf_destroy(headers);
  if (body) strbuf_destroy(body);
  return ret;
}

static int handle_listing_req(httpreq* req, char* datadir) {
  dumbfile* file = dumbfile_list(datadir);
  int ret = send_response(req, 200, build_listing(file));

  dumbfile_destroy(file);
  return ret;
}

/* we only support GETs :D */
int handle_client_req(int fd, char* datadir) {
  httpreq* req = httpreq_init(fd);
  int ret = 0;

  if (!req) {
    return -1;
  }

  INFO("HTTP request: [%s,%s,%s]", req->reqline.version, req->reqline.method, req->reqline.path);
#if LOG_LEVEL >= DEBUG
  for (httpheader* header = req->header; header; header = header->next) {
    DEBUG("%s=%s", header->key, header->val);
  }
#endif

  if (strcasecmp(req->reqline.path, "/") == 0) {
    ret = handle_listing_req(req, datadir);
  } else {
    ret = send_response(req, 404, 0);
  }

  httpreq_destroy(req);
  return ret;
}
