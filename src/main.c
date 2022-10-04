#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>

#include "log.h"
#include "server.h"

int main(int argc, char* argv[const static argc - 1]) {
  char* addr = 0;
  char* port = "80";
  int opt = 0;

  while ((opt = getopt(argc, argv, "a:p:")) != -1) {
    switch (opt) {
      case 'a':
        addr = optarg;
        break;
      case 'p':
        port = optarg;
        break;
    }
  }

  DEBUG("[addr: %s, port: %s]", addr, port);
  server_main(addr, port);
  
  return EXIT_SUCCESS;
}

