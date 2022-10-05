#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <linux/limits.h>

#include "log.h"
#include "server.h"

#define DEFAULT_DATA_DIR "/data"

int main(int argc, char* argv[const static argc - 1]) {
  char* addr = 0;
  char* port = 0;
  char* datadir = 0;
  int opt = 0;

  while ((opt = getopt(argc, argv, "a:p:d:")) != -1) {
    switch (opt) {
      case 'a':
        addr = optarg;
        break;
      case 'p':
        port = optarg;
        break;
    }
  }

  if (!port) {
    port = "80";
  }

  if (!datadir) {
    char cwd[PATH_MAX - strlen(DEFAULT_DATA_DIR)];
    getcwd(cwd, sizeof cwd);
    datadir = malloc(PATH_MAX);
    snprintf(datadir, PATH_MAX, "%s%s", cwd, DEFAULT_DATA_DIR);
  }

  DEBUG("[addr: %s, port: %s]", addr, port);
  server_main(addr, port);
  
  return EXIT_SUCCESS;
}

