#include <string_view>
#include <optional>
#include <vector>
#include <cstdio>

int cmd_main(int argc, char **argv) {
  if (argc < 4) {
    fprintf(stderr, "usage: %s refname oldrev newrev\n", argv[0]);
    return 1;
  }

  //
  return 0;
}
