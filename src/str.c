#include <string.h>
#include <ctype.h>

char* strtrim(char str[static 1]) {
  size_t len = strlen(str);
  size_t start = 0;
  size_t end = len;

  for (; start < len && isspace(str[start]); start++);
  for (; end > 0 && isspace(str[end-1]); end--);
 
  str[end] = 0;

  return str + start;
}
