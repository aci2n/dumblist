#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <errno.h>

#include "handler.h"
#include "http.h"
#include "log.h"
#include "dumb.h"
#include "str.h"

static strbuf* build_listing(dumbfile* list) {
  strbuf* sb = strbuf_init(512);

  strbuf_add(sb, 
      "<!DOCTYPE HTML><html><head><style>"
      "img {width: 50rem; max-width: 80vw}"
      "dt,dd {display: inline-block}"
      "dt {font-weight: bold}"
      "dt:after {content: ':'}"
      "dd {margin: 0; text-indent: 0.5rem}"
      "</style></head><body><header><h1>Listing</h1></header><main>");

  for (dumbfile* df = list; df; df = df->next) {
    strbuf_add(sb, "<article>");
    for (dumbfile_entry *e = df->entry; e; e = e->next) {
      if (strcasecmp(e->key, "title") == 0) {
        strbuf_printf(sb, "<h2>%s</h2>", e->val);
      } else if (strcasecmp(e->key, "image") == 0) {
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

static int handle_listing_req(httpreq* req, httpresp* resp, char* datadir) {
  dumbfile* file = dumbfile_list(datadir);
  int ret = 0;

  if (!file) {
    WARN("no dumbfiles in %s", datadir);
    resp->sc = 404;
  } else {
    resp->body = build_listing(file);
  }

  if (httpresp_send(resp) == -1) {
    ERROR("sending listing: %s", STRERROR);
    ret = -1;
  }

  if (file) dumbfile_destroy(file);
  if (resp->body) strbuf_destroy(resp->body);

  return ret;
}

static int handle_static_file_req(httpreq* req, httpresp* resp, char* datadir) {
  int ret = 0;

  // TODO: unsafe, could pass ../ to escape datadir
  stralloc(&resp->file_path, "%s%s", datadir, req->reqline.path);

  if (httpresp_send(resp) == -1) {
    ret = -1;
    ERROR("sending file %s: %s", resp->file_path, STRERROR);
  }

  free(resp->file_path);

  return ret;
}

/* we only support GETs :D */
int handle_client_req(int fd, char* datadir) {
  httpreq* req = httpreq_create(fd);
  httpresp* resp = httpresp_create(fd);
  int ret = 0;

  if (!req) {
    ERROR("could not read request: %s", STRERROR);
    ret = -1;
    goto done;
  }

  INFO("HTTP request: [%s,%s,%s]", req->reqline.version, req->reqline.method, req->reqline.path);
#if LOG_LEVEL >= LEVEL_TRACE
  for (httpheader* header = req->header; header; header = header->next) {
    TRACE("%s=%s", header->key, header->val);
  }
#endif

  if (strcasecmp(req->reqline.path, "/") == 0) {
    ret = handle_listing_req(req, resp, datadir);
  } else {
    ret = handle_static_file_req(req, resp, datadir);
  }

done:
  httpreq_destroy(req);
  httpresp_destroy(resp);

  return ret;
}
