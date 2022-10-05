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

// TODO: how the fuck do i get the content-length here
#define HTML_LIST_HEAD \
  "HTTP/1.1 200\r\n\r\n" \
  "<!DOCTYPE HTML>" \
  "<html>" \
  "<head>" \
  "</head>" \
  "<body>" \
    "<main>" \
      "<h1>Listing</h1>" \
      "<dl>" 
#define HTML_LIST_TAIL \
      "</dl>" \
    "</main>" \
  "</body>" \
  "</html>\r\n\r\n"

static void send_response(httpreq* req, dumbfile* list) {
  httpreq_send(req, HTML_LIST_HEAD);
  
  for (dumbfile* df = list; df; df = df->next) {
    httpreq_send(req, "<dt>");
    httpreq_send(req, "Title");
    httpreq_send(req, "<dt>");
    httpreq_send(req, "<dd>");
    httpreq_send(req, df->title);
    httpreq_send(req, "</dd>");

    for (dumbfile_entry* entry = df->entry; entry; entry = entry->next) {
      httpreq_send(req, "<dt>");
      httpreq_send(req, entry->key);
      httpreq_send(req, "</dt>");
      httpreq_send(req, "<dd>");
      httpreq_send(req, entry->val);
      httpreq_send(req, "</dd>");
    }
  }

  httpreq_send(req, HTML_LIST_TAIL);
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
