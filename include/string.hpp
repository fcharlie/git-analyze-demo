/////
#ifndef AZE_STRING_HPP
#define AZE_STRING_HPP
#include <string_view>
#include <cstring>

namespace aze {
inline bool starts_with(std::string_view sv, std::string_view prefix) {
  if (prefix.size() > sv.size()) {
    return false;
  }
  return memcmp(sv.data(), prefix.data(), prefix.size()) == 0;
}

inline bool ends_with(std::string_view sv, std::string_view suffix) {
  if (suffix.size() > sv.size()) {
    return false;
  }
  auto pos = sv.size() - suffix.size();
  return memcmp(sv.data() + pos, suffix.data(), suffix.size()) == 0;
}

} // namespace aze

#endif
