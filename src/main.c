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
  server_args args = { 
    .addr = 0, 
    .port = "8080", 
    .datadir = DEFAULT_DATA_DIR, 
  };

  for (int opt; (opt = getopt(argc, argv, "a:p:d:")) != -1;) {
    switch (opt) {
      case 'a':
        args.addr = optarg;
        break;
      case 'p':
        args.port = optarg;
        break;
      case 'd':
        args.datadir = optarg;
        break;
    }
  }

  DEBUG("[addr: %s, port: %s, datadir: %s]", args.addr, args.port, args.datadir);
  server_main(&args);
  
  return EXIT_SUCCESS;
}

