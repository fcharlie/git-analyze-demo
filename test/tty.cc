///
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifdef __GNUC__
int Printe(const char *format, ...)
    __attribute__((format(printf, 1, 2)));
#else
int Printe(const char *format, ...);
#endif

int BaseErrorWriteTTY(const void *buf, size_t len) {
  fwrite("\e[1;31m", 1, sizeof("\e[1;31m") - 1, stderr);
  auto l = fwrite(buf, 1, len, stderr);
  fwrite("\e[0m", 1, sizeof("\e[0m") - 1, stderr);
  return l;
}

int Printe(const char *format, ...) {
  char buf[8192];
  va_list ap;
  va_start(ap, format);
  auto l = vsnprintf(buf, 8192, format, ap);
  va_end(ap);
  if (isatty(STDERR_FILENO)) {
    return BaseErrorWriteTTY(buf, l);
  }
  return fwrite(buf, 1, l, stderr);
}

int main(int argc, char **argv) {
  if (argc < 2) {
    Printe("Argv: %s Less 2\n", argv[0]);
  } else {
    Printe("%s\n", argv[1]);
  }
  return 0;
}
