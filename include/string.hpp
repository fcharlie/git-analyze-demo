/////
#ifndef AZE_STRING_HPP
#define AZE_STRING_HPP
#include <string>
#include <string_view>
#include <cstring>
#include <cctype>

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

// inline int memcasecmp(const char* s1, const char* s2, size_t len) {
//   const unsigned char* us1 = reinterpret_cast<const unsigned char*>(s1);
//   const unsigned char* us2 = reinterpret_cast<const unsigned char*>(s2);
//
//   for (size_t i = 0; i < len; i++) {
//     const int diff =
//         int{static_cast<unsigned char>(absl::ascii_tolower(us1[i]))} -
//         int{static_cast<unsigned char>(absl::ascii_tolower(us2[i]))};
//     if (diff != 0) return diff;
//   }
//   return 0;
// }

inline bool ends_case_with(std::string_view sv, std::string_view suffix) {
  if (suffix.size() > sv.size()) {
    return false;
  }
  auto pos = sv.size() - suffix.size();
  auto P0 = sv.data() + pos;
  auto P1 = suffix.data();
  auto L = suffix.size();
  for (size_t I = 0; I < L; I++) {
    if (::tolower(P0[I]) != ::tolower(P1[I])) {
      return false;
    }
  }
  return true;
}

} // namespace aze

#endif
