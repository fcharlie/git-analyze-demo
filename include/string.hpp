/////
#ifndef AZE_STRING_HPP
#define AZE_STRING_HPP
#include <string>
#include <string_view>
#include <cstring>

namespace aze {

inline std::string catsv(std::initializer_list<std::string_view> pieces) {
  std::string result;
  size_t total_size = 0;
  for (const std::string_view piece : pieces) {
    total_size += piece.size();
  }
  result.resize(total_size);

  char *const begin = &*result.begin();
  char *out = begin;
  for (const std::string_view piece : pieces) {
    const size_t this_size = piece.size();
    memcpy(out, piece.data(), this_size);
    out += this_size;
  }
  return result;
}

inline std::string strcat() { return std::string(); }
inline std::string strcat(std::string_view sv) { return std::string(sv); }

template <typename... Args>
std::string strcat(std::string_view v0, const Args &... args) {
  return catsv({v0, args...});
}

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
