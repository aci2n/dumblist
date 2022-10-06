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
#include <sys/sendfile.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <assert.h>

#include "log.h"
#include "http.h"
#include "str.h"

#define HTTPREQ_PARTMAXLEN 1 << 10
#define HTTP_NEWLINE "\r\n"

httpreq* httpreq_create(int fd) {
  char recvbuf[512];
  size_t partlen = 1024;
  size_t partpos = 0;
  char* partbuf = malloc(partlen);
  bool reqline_done = false; 
  httpreq* req = malloc(sizeof *req);
  httpheader** header = &req->header;

  *req = (httpreq) { .fd = fd, };

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

          req->reqline.method = strtrim(method);
          req->reqline.path = strtrim(path);
          req->reqline.version = strtrim(version);

          reqline_done = true;
        } else {
          char* key = strtok(partbuf, ":");
          char* val = strtok(0, "\0");

          if (!key || !val) {
            ERROR("invalid header");
            goto error;
          }

          *header = malloc(sizeof **header);
          *(*header) = (httpheader) {
            .key = strtrim(key),
            .val = strtrim(val),
          };
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


void httpreq_destroy(httpreq* req) {
  if (!req) {
    return;
  }
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

static int send_buf(httpresp* resp, strbuf* buf) {
  size_t len = strbuf_len(buf);
  char* data = strbuf_data(buf);
  size_t sent = 0;

  while (sent < len) {
    size_t n = send(resp->fd, data + sent, len - sent, 0);

    if (n == -1) {
      ERROR("send: %s", strerror(errno));
      return errno;
    }

    sent += n;
    TRACE("sent %lu bytes (%lu/%lu)", n, sent, len);
  }

  return 0;
}

static int send_file(httpresp* resp) {
  size_t const len = resp->content_length;
  int const fd = fileno(resp->file);
  size_t sent = 0;

  assert(fd != -1);

  while (sent < len) {
    size_t const n = sendfile(resp->fd, fd, 0, len - sent);

    if (n == -1) {
      ERROR("sendfile: %s", STRERROR);
      return -1;
    }

    sent += n;
    TRACE("sendfile %lu bytes (%lu/%lu) ", n, sent, len);
  }

  return 0;
}

httpresp* httpresp_create(int fd) {
  httpresp* resp = malloc(sizeof *resp);
  *resp = (httpresp) {
    .fd = fd,
    .sc = 200,
    .content_type = CT_ASCII,
  };
  return resp;
}

static strbuf* build_headers(httpresp* resp) {
  strbuf* sb = strbuf_init(128);
  char* content_type;

  strbuf_printf(sb, "HTTP/1.1 %u" HTTP_NEWLINE, resp->sc);
  switch (resp->content_type) {
    case CT_JPG:
      content_type = "image/jpg";
      break;
    case CT_ASCII:
    default:
      content_type = "text/html; charset=us-ascii";
      break;
  }
  strbuf_printf(sb, "content-type: %s" HTTP_NEWLINE, content_type);
  strbuf_printf(sb, "content-length: %lu" HTTP_NEWLINE, resp->content_length);
  strbuf_add(sb, HTTP_NEWLINE);

  TRACE("response headers: %s", strbuf_data(sb));

  return sb;
}

int httpresp_send(httpresp* resp) {
  strbuf* headers = 0;
  int ret = 0;

  if (resp->body) {
    resp->content_length = strbuf_len(resp->body);
  } else if (resp->file) {
    struct stat st;
    int file_fd = 0;

    if ((file_fd = fileno(resp->file)) == -1) {
      ERROR("fileno: %s", STRERROR);
      ret = -1;
      goto done;
    }

    if (fstat(file_fd, &st) == -1) {
      ERROR("fstat: %s", STRERROR);
      ret = -1;
      goto done;
    }

    resp->content_length = st.st_size;
  } else {
    resp->content_length = 0;
  }

  headers = build_headers(resp);

  if (send_buf(resp, headers) == -1) {
    ret = -1;
    goto done;
  }

  if (resp->body) {
    if (send_buf(resp, resp->body) == -1) {
      ret = -1;
      goto done;
    }
  } else if (resp->file) {
    if (send_file(resp) == -1) {
      ret = -1;
      goto done;
    }
  }

done:
  strbuf_destroy(headers);
  return ret;
}

void httpresp_destroy(httpresp* resp) {
  if (resp) {
    free(resp);
  }
}
