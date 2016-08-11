////
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>

#include <Windows.h>
#include <io.h>

class WCharacters {
private:
  wchar_t *wstr;
  uint32_t len;

public:
  WCharacters(const char *str, size_t size) : wstr(nullptr) {
    if (str == nullptr)
      return;
    int unicodeLen = ::MultiByteToWideChar(CP_UTF8, 0, str, size, NULL, 0);
    if (unicodeLen == 0)
      return;
    wstr = new wchar_t[unicodeLen + 1];
    if (wstr == nullptr)
      return;
    wstr[unicodeLen] = 0;
    ::MultiByteToWideChar(CP_UTF8, 0, str, -1, (LPWSTR)wstr, unicodeLen);
    len = unicodeLen;
  }
  const wchar_t *Get() {
    if (!wstr)
      return nullptr;
    return const_cast<const wchar_t *>(wstr);
  }
  uint32_t Length() const { return len; }
  ~WCharacters() {
    if (wstr)
      delete[] wstr;
  }
};

bool IsUnderConhost() {
  HANDLE hStderr = reinterpret_cast<HANDLE>(_get_osfhandle(_fileno(stderr)));
  return GetFileType(hStderr) == FILE_TYPE_CHAR;
}

// bool IsUnderConhost() {
//   return _isatty(_fileno(stderr)) != 0;
// }

int BaseErrorWriteConhost(const char *buf, size_t len) {
  //
  HANDLE hConsole = GetStdHandle(STD_ERROR_HANDLE);
  CONSOLE_SCREEN_BUFFER_INFO csbi;
  GetConsoleScreenBufferInfo(hConsole, &csbi);
  WORD oldColor = csbi.wAttributes;
  WORD newColor = (oldColor & 0xF0) | FOREGROUND_INTENSITY | FOREGROUND_RED;
  SetConsoleTextAttribute(hConsole, newColor);
  DWORD dwWrite;
  WCharacters wstr(buf, len);
  WriteConsoleW(hConsole, wstr.Get(), wstr.Length(), &dwWrite, nullptr);
  SetConsoleTextAttribute(hConsole, oldColor);
  return 0;
}

int BaseErrorMessagePrint(const char *format, ...) {
  static bool conhost_ = IsUnderConhost();
  char buf[16348];
  va_list ap;
  va_start(ap, format);
  auto l = vsnprintf(buf, 16348, format, ap);
  va_end(ap);
  if (conhost_) {
    return BaseErrorWriteConhost(buf, l);
  }
  return fwrite(buf, 1, l, stderr);
}

int main() {
  ////
  if (IsUnderConhost()) {
    MessageBoxW(nullptr, L"Run Under ConsoleHost", L"Title", MB_OK);
  } else {
    MessageBoxW(nullptr, L"Not ConsoleHost", L"Title", MB_OK);
  }
  BaseErrorMessagePrint("Check error, block\n");
  return 0;
}
