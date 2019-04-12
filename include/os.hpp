//////////////
#ifndef GIT_ANALYZE_OS_HPP
#define GIT_ANALYZE_OS_HPP
#include <string>
#include <string_view>
#include <optional>
#include <vector>
#include <uv.h>

namespace os {

#ifdef _WIN32
constexpr const char PathSeparator = '\\';
constexpr const char PathUnixSeparator = '/';
constexpr const size_t PathMax = 0x8000;
inline bool IsPathSeparator(char c) {
  return c == PathSeparator || c == PathUnixSeparator;
}
#else
constexpr const char PathSeparator = '/';
constexpr const size_t PathMax = 4096;
inline bool IsPathSeparator(char c) { return c == PathSeparator; }
#endif

inline bool Executable(std::string &exe) {
  size_t size = PathMax;
  exe.resize(size);
  if (uv_exepath(exe.data(), &size) != 0) {
    return false;
  }
  exe.resize(size);
  return false;
}

inline std::string Getwd() {
  size_t size = PathMax;
  std::string dir;
  dir.resize(size);
  if (uv_cwd(dir.data(), &size) != 0) {
    return ".";
  }
  dir.resize(size);
  return dir;
}

inline bool PathRemoveFileSpec(std::string &path) {
  if (path.empty()) {
    return false;
  }
  auto p = path.c_str();
  auto end = p + path.size();
  while (p < end--) {
    if (IsPathSeparator(*end)) {
      auto l = end - p;
      path.erase(l == 0 ? 1 : l);
      return true;
    }
  }
  return true;
}

inline std::string GetEnv(std::string_view key) {
  std::string ev;
  ev.resize(128);
  size_t l = 128;
  switch (uv_os_getenv(key.data(), ev.data(), &l)) {
  case 0:
    ev.resize(l);
    return ev;
  case UV_ENOBUFS:
    break;
  case UV_ENOENT:
  /*fall*/
  default:
    return "";
  }
  l++;
  ev.resize(l);
  if (uv_os_getenv(key.data(), ev.data(), &l) != 0) {
    return "";
  }
  return ev;
}
} // namespace os

#endif
