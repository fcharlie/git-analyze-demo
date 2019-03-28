////////////////
#include <cstdio>
#include <git2.h>

int main(int argc, char const *argv[]) {
  if (argc < 3) {
    fprintf(stderr, "usage: %s gitdir commit\n", argv[0]);
    return 1;
  }
  return 0;
}
