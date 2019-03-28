///////////
#ifndef OS_ENV_IMPL_HPP
#define OS_ENV_IMPL_HPP
#include <string_view>
#include <cstdio>
#include <cstdlib>
#include <cerrno>

#ifdef _WIN32
#include <Windows.h>
typedef std::wstring_view string_view_t;
#define _X(x) L##x
#else
typedef std::string_view string_view_t;
#define _X(x) x
#endif

namespace aze {
template <typename Integer> Integer GetEnv(string_view_t key, Integer value) {
#ifdef _WIN32
  wchar_t buf[64];
  if (GetEnvironmentVariableW(key.data(), buf, 64) != 0) {
    wchar_t *c;
    auto i = wcstoll(buf, &c, 10);
    if (errno != 0) {
      return value;
    }
    return static_cast<Integer>(i);
  }
#else
  auto va = std::getenv(key.data());
  if (va == nullptr) {
    return value;
  }
  char *c = nullptr;
  auto i = std::strtoll(va, &c, 10);
  if (errno != 0) {
    return static_cast<Integer>(i);
  }
#endif
  return value;
}

} // namespace aze

#endif
