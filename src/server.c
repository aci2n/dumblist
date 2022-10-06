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
#include "handler.h"

#define BACKLOG 10

static void* get_in_addr(struct sockaddr* sa) {
  if (sa->sa_family == AF_INET) {
    return &(((struct sockaddr_in*)sa)->sin_addr);
  }
  return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

static void get_addr_str(struct sockaddr* sa, size_t len, char buf[static len]) {
  inet_ntop(sa->sa_family, get_in_addr(sa), buf, len);
}

static int get_server_fd(char* addr, char* port) {
  int gai_status = 0;
  struct addrinfo* servinfo = 0;
  struct addrinfo hints = {
    .ai_family = AF_UNSPEC,
    .ai_socktype = SOCK_STREAM,
    .ai_flags = AI_PASSIVE,
  };
  int fd = -1;
  int yes = 1;

  DEBUG("will try to connect to: [addr=%s,port=%s]", addr, port);

  if ((gai_status = getaddrinfo(addr, port, &hints, &servinfo)) == -1) {
    ERROR("getaddrinfo: %s", gai_strerror(gai_status));
    return -1;
  }

  for (struct addrinfo* ai = servinfo; ai; ai = ai->ai_next) {
    if ((fd = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol)) == -1) {
      DEBUG("socket: %s", strerror(errno));
      continue;
    }

    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof yes) == -1) {
      ERROR("setsockopt: %s", strerror(errno));
      close(fd);
      fd = -1;
      break;
    }

    if (bind(fd, ai->ai_addr, ai->ai_addrlen) == -1) {
      WARN("bind: %s", strerror(errno));
      close(fd);
      fd = -1;
      continue;
    }
    
    if (listen(fd, BACKLOG) == -1) {
      WARN("listen: %s", strerror(errno));
      close(fd);
      fd = -1;
      continue;
    }

#if LOG_LEVEL >= LEVEL_INFO
    char server_addr[INET6_ADDRSTRLEN];
    get_addr_str(ai->ai_addr, sizeof server_addr, server_addr);
    INFO("listening on %s:%s", server_addr, port);
#endif

    break;
  }

  freeaddrinfo(servinfo);

  return fd;
}

void server_main(char* addr, char* port, char* datadir) {
  int server_fd = get_server_fd(addr, port);

  if (server_fd == -1) {
    ERROR("could not bind to address");
    return;
  }

  // TODO: handle SIGINT?
  while (true) {
    struct sockaddr_storage client_sa = {0};
    socklen_t client_sa_size = sizeof client_sa;
    int client_fd = accept(server_fd, (struct sockaddr*)&client_sa, &client_sa_size);

    if (client_fd == -1) {
      ERROR("client_fd: %s", strerror(errno));
      continue;
    }

#if LOG_LEVEL >= LEVEL_INFO
    char client_addr[INET6_ADDRSTRLEN];   
    get_addr_str((struct sockaddr*)&client_sa, sizeof client_addr, client_addr);
    INFO("request from: %s", client_addr);
#endif

#if SERVER_FORK == 1
    if (!fork()) {
      // in child process
      close(server_fd);
      handle_client_req(client_fd, datadir);
      close(client_fd);
      exit(0);
    }
    close(client_fd);
#else
    handle_client_req(client_fd, datadir);
    close(client_fd);
#endif
  } 
}
