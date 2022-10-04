#ifndef LOG_H
#define LOG_H 1

#include <errno.h>
#include <stdbool.h>

#define LEVEL_ERROR 0
#define LEVEL_WARN 1
#define LEVEL_INFO 2
#define LEVEL_DEBUG 3
#define LEVEL_TRACE 4

#ifndef LOG_LEVEL
  #define LOG_LEVEL LEVEL_WARN
#endif

/* preprocessor magic */

#define STRINGIFY0(X) #X
#define STRINGIFY(X) STRINGIFY0(X)

#define ONLY_FIRST0(_0, ...) _0
#define ONLY_FIRST(...) ONLY_FIRST0(__VA_ARGS__, 0)

#define EXCLUDE_FIRST0(_0, ...) __VA_ARGS__
#define EXCLUDE_FIRST(...) EXCLUDE_FIRST0(__VA_ARGS__, 0)

#define LOG0(LEVEL, LEVEL_STR, FORMAT, ...) \
  do { \
    if (LEVEL <= LOG_LEVEL) { \
      int perrno = errno; \
      fprintf(stderr, \
          LEVEL_STR " [" __FILE__ ":" STRINGIFY(__LINE__) " %s()] " FORMAT "\n", \
           __func__, ##__VA_ARGS__); \
      errno = perrno; \
    } \
  } while (false);
#define LOG(LEVEL, LEVEL_STR, ...) \
  LOG0(LEVEL, LEVEL_STR, ONLY_FIRST(__VA_ARGS__) "%.0d", EXCLUDE_FIRST(__VA_ARGS__))

#define ERROR(...) LOG(LEVEL_ERROR, "ERROR", __VA_ARGS__);
#define WARN(...) LOG(LEVEL_WARN, "WARN", __VA_ARGS__);
#define INFO(...) LOG(LEVEL_INFO, "INFO", __VA_ARGS__);
#define DEBUG(...) LOG(LEVEL_DEBUG, "DEBUG", __VA_ARGS__);
#define TRACE(...) LOG(LEVEL_TRACE, "TRACE", __VA_ARGS__);

#endif
