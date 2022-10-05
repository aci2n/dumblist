#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <linux/limits.h>

#include "log.h"
#include "server.h"

#define DEFAULT_DATA_DIR "./data"

int main(int argc, char* argv[const static argc - 1]) {
  char* addr = 0;
  char* port = "80";
  char* datadir = DEFAULT_DATA_DIR;
  int opt = 0;

  while ((opt = getopt(argc, argv, "a:p:d:")) != -1) {
    switch (opt) {
      case 'a':
        addr = optarg;
        break;
      case 'p':
        port = optarg;
        break;
      case 'd':
        datadir = optarg;
        break;
    }
  }

  DEBUG("[addr: %s, port: %s, datadir: %s]", addr, port, datadir);
  server_main(addr, port, datadir);
  
  return EXIT_SUCCESS;
}

