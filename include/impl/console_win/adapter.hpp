//////////
#ifndef AZE_CONSOLE_WIN_ADAPTER_HPP
#define AZE_CONSOLE_WIN_ADAPTER_HPP
#ifndef _WINDOWS_
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN //
#endif
#include <windows.h>
#endif

#include <functional>

namespace aze {
using ssize_t = SSIZE_T;
namespace details {
enum adaptermode_t : int {
  AdapterFile,
  AdapterConsole,
  AdapterConsoleTTY,
  AdapterTTY
};

class adapter {
public:
  adapter(const adapter &) = delete;
  adapter &operator=(const adapter &) = delete;
  ~adapter() {
    //
  }
  static adapter &instance() {
    static adapter adapter_;
    return adapter_;
  }
  ssize_t adapterwrite(int color, const char *data, size_t len) {
    switch (at) {
    case AdapterFile:
      return writefile(color, data, len);
    case AdapterConsole:
      return writeoldconsole(color, data, len);
    case AdapterConsoleTTY:
      return writeconsole(color, data, len);
    case AdapterTTY:
      return writetty(color, data, len);
    }
    return -1;
  }
  bool changeout(bool isstderr) {
    out = isstderr ? stderr : stdout;
    return out == stderr;
  }

private:
  adapter();
  ssize_t writefile(int color, const char *data, size_t len);
  ssize_t writeoldconsole(int color, const char *data, size_t len);
  ssize_t writeconsole(int color, const char *data, size_t len);
  ssize_t writetty(int color, const char *data, size_t len);
  //
  int WriteConsoleInternal(const char *buffer, size_t len);
  adaptermode_t at{AdapterConsole};
  HANDLE hConsole{nullptr};
  FILE *out{stdout};
};
} // namespace details
} // namespace aze

#endif
