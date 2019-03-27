///////
#ifndef GA_OS_WINDOWS_HPP
#define GA_OS_WINDOWS_HPP
#include <string>

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

} // namespace os

#endif
