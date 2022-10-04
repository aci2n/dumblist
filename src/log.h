#ifndef LOG_H
#define LOG_H 1

#include <errno.h>
#include <stdbool.h>

#define STRINGIFY0(X) #X
#define STRINGIFY(X) STRINGIFY0(X)

#define LEVEL_ERROR 0
#define LEVEL_WARN 1
#define LEVEL_INFO 2
#define LEVEL_DEBUG 3
#define LEVEL_TRACE 4

#ifndef LOG_LEVEL
  #define LOG_LEVEL LEVEL_WARN
#endif

#define LOG(LEVEL, LEVEL_STR, FORMAT, ...) \
  do { \
    if (LEVEL <= LOG_LEVEL) { \
      int perrno = errno; \
      fprintf(stderr, \
          LEVEL_STR " [" __FILE__ ":" STRINGIFY(__LINE__) " %s()] " FORMAT "\n", \
           __func__, __VA_ARGS__); \
      errno = perrno; \
    } \
  } while (false);

#define ERROR(FORMAT, ...) LOG(LEVEL_ERROR, "ERROR", FORMAT, __VA_ARGS__);
#define WARN(FORMAT, ...) LOG(LEVEL_WARN, "WARN", FORMAT, __VA_ARGS__);
#define INFO(FORMAT, ...) LOG(LEVEL_INFO, "INFO", FORMAT, __VA_ARGS__);
#define DEBUG(FORMAT, ...) LOG(LEVEL_DEBUG, "DEBUG", FORMAT, __VA_ARGS__);
#define TRACE(FORMAT, ...) LOG(LEVEL_TRACE, "TRACE", FORMAT, __VA_ARGS__);

#endif
