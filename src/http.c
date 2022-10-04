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

#include "log.h"
#include "http.h"

#define HTTPREQ_PARTMAXLEN 1 << 10

httpreq* httpreq_init(int fd) {
  char recvbuf[512];
  size_t partlen = 1024;
  size_t partpos = 0;
  char* partbuf = malloc(partlen);
  bool reqline_done = false; 
  httpreq* req = malloc(sizeof *req);
  httpheader** header = &req->header;

  if (!req || !partbuf) {
    ERROR("malloc httpreq");
    goto error;
  }

  while (true) {
    size_t n = recv(fd, recvbuf, sizeof recvbuf, 0);
    TRACE("recv: %lu bytes", n);

    if (n == -1) {
      ERROR("read: %s", strerror(errno));
      goto error;
    }

    if (n == 0) {
      WARN("client closed connection");
      goto error;
    }

    for (size_t i = 0; i < n; i++) {
      if (recvbuf[i] == '\n' && partpos > 0 && partbuf[partpos - 1] == '\r') {
        if (partpos == 1) {
          goto done;
        }

        partbuf[partpos - 1] = 0;

        if (!reqline_done) {
          char* method = strtok(partbuf, " ");
          char* path = strtok(0, " ");
          char* version = strtok(0, "\0");

          if (!method || !path || !version) {
            ERROR("invalid reqline");
            goto error;
          }

          req->reqline.method = strdup(method);
          req->reqline.path = strdup(path);
          req->reqline.version = strdup(version);

          reqline_done = true;
        } else {
          char* key = strtok(partbuf, ":");
          char* val = strtok(0, "\0");

          if (!key || !val) {
            ERROR("invalid header");
            goto error;
          }

          *header = malloc(sizeof *header);
          *(*header) = (httpheader) { .key = strdup(key), .val = strdup(val), };
          header = &(*header)->next;
        }

        partpos = 0;
        continue;
      }

      if (partpos == partlen) {
        if (partlen >= HTTPREQ_PARTMAXLEN) {
          ERROR("exceeded max part len");
          goto error;
        }

        partlen <<= 1;
        partbuf = realloc(partbuf, partlen);

        if (!partbuf) {
          ERROR("realloc part buf: %s", strerror(errno));
          goto error;
        }
      }

      partbuf[partpos++] = recvbuf[i];
    }
  }

error:
  free(partbuf);
  free(req);
  return 0;

done:
  free(partbuf);
  return req;
}


void httpreq_destroy(httpreq req[static 1]) {
  for (httpheader* header = req->header; header;) {
    httpheader* next = header->next;
    free(header->key);
    free(header->val);
    free(header);
    header = next;
  }
  free(req->reqline.method);
  free(req->reqline.path);
  free(req->reqline.version);
  free(req);
}
