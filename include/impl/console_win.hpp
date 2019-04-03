///////////////////
#ifndef IMPL_CONSOLE_WIN_HPP
#define IMPL_CONSOLE_WIN_HPP
#include <cstdio>
#include <string_view>
#ifndef _WINDOWS_
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN //
#endif
#include <Windows.h>
#endif

#ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
#define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
#endif

namespace aze {
namespace internal {
//
enum console_mode {
  CONSOLE_FILE = 0, // File
  CONSOLE_TTY,      // Mintty like
  CONSOLE_CONHOST,  // Conhost
  CONSOLE_PTY       // Windows 10 PTY
};

inline bool EnableVTMode(HANDLE hFile) {
  DWORD dwMode = 0;
  if (!GetConsoleMode(hFile, &dwMode)) {
    return false;
  }
  dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
  if (!SetConsoleMode(hFile, dwMode)) {
    return false;
  }
  return true;
}

inline int FileHandleMode(HANDLE hFile, bool &vt) {
  if (hFile == INVALID_HANDLE_VALUE) {
    return CONSOLE_FILE;
  }
  if (GetFileType(hFile) == FILE_TYPE_DISK) {
    return CONSOLE_FILE;
  }
  if (GetFileType(hFile) == FILE_TYPE_CHAR) {
    vt = EnableVTMode(hFile);
    return CONSOLE_CONHOST;
  }
  return CONSOLE_TTY;
}

inline std::wstring utf8towide(std::string_view str) {
  std::wstring wstr;
  auto N =
      MultiByteToWideChar(CP_UTF8, 0, str.data(), (int)str.size(), nullptr, 0);
  if (N > 0) {
    wstr.resize(N);
    MultiByteToWideChar(CP_UTF8, 0, str.data(), (int)str.size(), &wstr[0], N);
  }
  return wstr;
}

class console_write_adapter {
public:
  console_write_adapter(const console_write_adapter &) = delete;
  console_write_adapter &operator=(const console_write_adapter &) = delete;
  int write(FILE *out, std::string_view msg) {
    int mode = (out == stderr ? stderrmode : stdoutmode);
    switch (mode) {
    case CONSOLE_TTY:
    case CONSOLE_FILE:
    case CONSOLE_PTY:
      break;
    case CONSOLE_CONHOST: {
      if (out == stderr) {
        return writeconsole(hStderr, stderrvt, msg);
      }
      return writeconsole(hStdout, stdoutvt, msg);
    }
    }
    return (int)fwrite(msg.data(), 1, msg.size(), out);
  }
  static console_write_adapter &instance() {
    static console_write_adapter adapter;
    return adapter;
  }

private:
  HANDLE hStderr;
  HANDLE hStdout;
  int stderrmode{0};
  int stdoutmode{0};
  bool stderrvt{false};
  bool stdoutvt{false};
  console_write_adapter() { initialize(); }
  bool initialize() {
    hStderr = GetStdHandle(STD_OUTPUT_HANDLE);
    hStdout = GetStdHandle(STD_ERROR_HANDLE);
    stderrmode = FileHandleMode(hStderr, stderrvt);
    stdoutmode = FileHandleMode(hStdout, stdoutvt);
    return false;
  }
  int writeconsole(HANDLE hFile, bool vt, std::string_view sv) {
    if (vt) {
      auto wsv = utf8towide(sv);
      DWORD dwWrite = 0;
      WriteConsoleW(hFile, wsv.data(), (DWORD)wsv.size(), &dwWrite, nullptr);
      return (int)dwWrite;
    }
    // unsupport now
    auto wsv = utf8towide(sv);
    DWORD dwWrite = 0;
    WriteConsoleW(hFile, wsv.data(), (DWORD)wsv.size(), &dwWrite, nullptr);
    return (int)dwWrite;
  }
};

inline int console_write(FILE *out, std::string_view msg) {
  if (out != stdout && out != stderr) {
    return (int)fwrite(msg.data(), 1, msg.size(), out);
  }
  return console_write_adapter::instance().write(out, msg);
}
} // namespace internal
} // namespace aze

#endif
