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

#include "analyze.hpp"

std::size_t EnvLimitSize() {
  auto str = getenv(GIT_ANALYZE_LIMITSIZE);
  if (str) {
    char *c;
    auto l = strtol(str, &c, 10);
    if (l > 0 && l < 10000) {
      return l * MBSIZE;
    }
  }
  return 100 * MBSIZE;
}

std::size_t EnvWarnSize() {
  auto str = getenv(GIT_ANALYZE_WARNSIZE);
  if (str) {
    char *c;
    auto l = strtol(str, &c, 10);
    if (l > 0 && l < 10000) {
      return l * MBSIZE;
    }
  }
  return 50 * MBSIZE;
}

std::int64_t EnvTimeout() {
  auto str = getenv(GIT_ANALYZE_TIMEOUT);
  if (str) {
    char *c;
    auto l = strtol(str, &c, 10);
    if (l > 0 && l < 7200) {
      return l;
    }
  }
  return -1;
}
