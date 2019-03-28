///////
#ifndef GA_OS_WINDOWS_HPP
#define GA_OS_WINDOWS_HPP
#include <string>
#include <string_view>
#include <optional>
#include <vector>

namespace os {
inline bool Executable(std::string &exe) {
  (void)exe;
  // NOTIMPL
  return false;
}

inline bool PathRemoveFileSpec(std::string &path) {
  if (path.empty()) {
    return false;
  }
  auto p = path.c_str();
  auto end = p + path.size();
  while (p < end--) {
    if (*end == '/' || *end == '\\') {
      auto l = end - p;
      path.erase(l == 0 ? 1 : l);
      return true;
    }
  }
  return false;
}

constexpr const char PathSeparator = '\\';

inline std::string Getwd() {
  // NOTIMPL
  return ".";
}

inline std::optional<std::string> Gitdir(std::string_view sv) {
  /// NOTIMPL
  return std::make_optional(std::string(sv));
}

} // namespace os

#endif
