#ifndef SERVER_H
#define SERVER_H 1

typedef struct server_args server_args;
struct server_args {
  char* addr;
  char* port;
  char* datadir;

};

void server_main(server_args* args);

#endif
