///////////////////
#ifndef IMPL_CONSOLE_WIN_HPP
#define IMPL_CONSOLE_WIN_HPP
#include <cstdio>
#include <string_view>

namespace aze {
namespace internal {
//
inline int console_write(FILE *out, std::string_view msg) {
  if (out != stdout || out != stderr) {
    return (int)fwrite(msg.data(), 1, msg.size(), out);
  }
  // CYGWIN like tty
  // Windows console --> UTF-8 to UTF-16
  return 0;
}
} // namespace internal
} // namespace aze

#endif
