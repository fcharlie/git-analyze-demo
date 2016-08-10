
#include <cstring>
#include <cstdio>

inline bool IsArg(const char *candidate, const char *longname, size_t n,
                  const char **off) {
  auto l = strlen(candidate);
  if (l < n)
    return false;
  if (strncmp(candidate, longname, n) == 0) {
    if (l > n && candidate[n] == '=') {
      *off = candidate + n + 1;
    } else {
      *off = nullptr;
    }
    return true;
  }
  return false;
}

int main(int argc, char **argv) {
  //
  const char *c = nullptr;
  for (auto i = 1; i < argc; i++) {
    // const char *arg = argv[i];
    if (IsArg(argv[i], "--dir", sizeof("--dir") - 1, &c)) {
      if (c) {
        fprintf(stderr, "DIR: %s\n", c);
      } else {
        if (++i < argc) {
          fprintf(stderr, "DIR: %s\n", argv[i]);
        }
      }
    } else if (IsArg(argv[i], "--limit", sizeof("--limit") - 1, &c)) {
      if (c) {
        fprintf(stderr, "LIMIT %s\n", c);
      } else {
        if (++i < argc) {
          fprintf(stderr, "LIMIT: %s\n", argv[i]);
        }
      }
    }
  }
  return 0;
}
