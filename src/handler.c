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

done:
  httpreq_destroy(req);
  return 0;
}
