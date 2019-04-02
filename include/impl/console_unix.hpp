//
#ifndef CONSOLE_UNIX_HPP
#define CONSOLE_UNIX_HPP
///////////
#include <string>
#include <string_view>

namespace aze {

enum FcColor {
  Black = 0,
  DarkBlue = 1,
  DarkGreen = 2,
  DarkCyan = 3,
  DarkRed = 4,
  DarkMagenta = 5,
  DarkYellow = 6,
  Gray = 7,
  DarkGray = 8,
  Blue = 9,
  Green = 10,
  Cyan = 11,
  Red = 12,
  Magenta = 13,
  Yellow = 14,
  White = 15
};

template <typename T> T Argument(T value) noexcept { return value; }
template <typename T>
T const *Argument(std::basic_string<T> const &value) noexcept {
  return value.c_str();
}

template <typename T>
T const *Argument(std::basic_string_view<T> const &value) noexcept {
  return value.data();
}

template <typename... Args>
int StringPrint(char *const buffer, size_t const bufferCount,
                char const *const format, Args const &... args) noexcept {
  int const result = sprintf(buffer, bufferCount, format, Argument(args)...);
  // ASSERT(-1 != result);
  return result;
}

template <typename... Args> ssize_t Print(const char *format, Args... args) {
  std::string buffer;
  size_t size = StringPrint(nullptr, 0, format, args...);
  buffer.resize(size);
  size = StringPrint(&buffer[0], buffer.size() + 1, format, args...);
  return fwrite(buffer.data(), 1, size, stderr);
}

template <typename... Args>
ssize_t Verbose(bool verbose, const char *format, Args... args) {
  if (!verbose) {
    return 0;
  }
  std::string buffer;
  constexpr const char start[] = "\x1b[31m";
  constexpr const char end[] = "\x1b[0m\n";
  constexpr size_t k = sizeof(start) - 1;
  constexpr size_t sl = sizeof(end) - 1;
  size_t size = StringPrint(nullptr, 0, format, args...);
  buffer.resize(size + k + sl);
  buffer.assign("\x1b[33m"); //"\x1b[0m"
  size =
      StringPrint(buffer.data() + k, buffer.size() - k - sl, format, args...);
  buffer.resize(size + k);
  buffer.append(end);
  return fwrite(buffer.data(), 1, size, stderr);
}
} // namespace aze

#endif
