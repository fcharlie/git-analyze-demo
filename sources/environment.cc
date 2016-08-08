//// This is parse environment
////
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#ifdef _WIN32
#include <io.h>
#else
#include <unistd.h>
#endif

#define MBSIZE (1UL << 20)

std::size_t LimitSize() {
  auto str = getenv("GIT_LIMIT_SIZE");
  if (str) {
    char *c;
    auto l = strtol(str, &c, 10);
    if (l > 0 && l < 10000) {
      return l * MBSIZE;
    }
  }
  return 100 * MBSIZE;
}

std::size_t WarnSize() {
  auto str = getenv("GIT_WARN_SIZE");
  if (str) {
    char *c;
    auto l = strtol(str, &c, 10);
    if (l > 0 && l < 10000) {
      return l * MBSIZE;
    }
  }
  return 50 * MBSIZE;
}
