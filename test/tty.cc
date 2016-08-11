///
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char **argv) {
  char buf[4096];
  auto l = snprintf(buf, 4096, "\e[1;31m %s \e[0m\n", argv[0]);
  write(STDERR_FILENO, buf, l);
  return 0;
}
