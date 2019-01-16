/*
* Console.cc
* git-analyze Pal
* author: Force.Charlie
* Date: 2016.08
* Copyright (C) 2019. GITEE.COM. All Rights Reserved.
*/
#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <cstdarg>

#ifdef _WIN32
#include <windows.h>
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

bool IsUnderConhost(FILE *fp) {
  HANDLE hStderr = reinterpret_cast<HANDLE>(_get_osfhandle(_fileno(fp)));
  return GetFileType(hStderr) == FILE_TYPE_CHAR;
}

// bool IsUnderConhost() {
//   return _isatty(_fileno(stderr)) != 0;
// }

/// by default, Mintty and Cygwin Term ,console process  not run under conhost
/// check env has 'TERM' TERM=xterm or LANG=zh_CN.UTF-8
bool IsWindowsTTY() {
  ///
  if (GetEnvironmentVariableW(L"TERM", NULL, 0) == 0) {
    if (GetLastError() == ERROR_ENVVAR_NOT_FOUND)
      return false;
  }
  return true;
}

int BaseWriteTTY(bool bold, int color, const char *buf, size_t len) {
  if (bold) {
    fprintf(stderr, "\33[1;%dm", color);
  } else {
    fprintf(stderr, "\33[%dm", color);
  }
  auto l = fwrite(buf, 1, len, stderr);
  fwrite("\33[0m", 1, sizeof("\33[0m") - 1, stderr);
  return l;
}

int BaseWriteConhost(const char *buf, size_t len) {
  HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
  WCharacters wstr(buf, len);
  DWORD dwWrite;
  WriteConsoleW(hConsole, wstr.Get(), wstr.Length(), &dwWrite, nullptr);
  return dwWrite;
}

int BaseColorWriteConhost(WORD dColor, const char *buf, size_t len) {
  HANDLE hConsole = GetStdHandle(STD_ERROR_HANDLE);
  CONSOLE_SCREEN_BUFFER_INFO csbi;
  GetConsoleScreenBufferInfo(hConsole, &csbi);
  WORD oldColor = csbi.wAttributes;
  WORD newColor = (oldColor & 0xF0) | dColor;
  SetConsoleTextAttribute(hConsole, newColor);
  DWORD dwWrite;
  WCharacters wstr(buf, len);
  WriteConsoleW(hConsole, wstr.Get(), wstr.Length(), &dwWrite, nullptr);
  SetConsoleTextAttribute(hConsole, oldColor);
  return dwWrite;
}

int Printe(const char *format, ...) {
  static bool conhost_ = IsUnderConhost(stderr);
  static bool wintty_ = IsWindowsTTY();
  char buf[16348];
  va_list ap;
  va_start(ap, format);
  auto l = vsnprintf(buf, 16348, format, ap);
  va_end(ap);
  if (conhost_) {
    WORD color = FOREGROUND_INTENSITY | FOREGROUND_RED;
    return BaseColorWriteConhost(color, buf, l);
  }
  if (wintty_) {
    return BaseWriteTTY(false, 31, buf, l);
  }
  return fwrite(buf, 1, l, stderr);
}

int Printw(const char *format, ...) {
  static bool conhost_ = IsUnderConhost(stderr);
  static bool wintty_ = IsWindowsTTY();
  char buf[16348];
  va_list ap;
  va_start(ap, format);
  auto l = vsnprintf(buf, 16348, format, ap);
  va_end(ap);
  if (conhost_) {
    WORD color = FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_BLUE;
    return BaseColorWriteConhost(color, buf, l);
  }
  if (wintty_) {
    return BaseWriteTTY(true, 33, buf, l);
  }
  return fwrite(buf, 1, l, stderr);
}
//// To complete
int Print(const char *format, ...) {
  static bool conhost_ = IsUnderConhost(stdout);
  char buf[16348];
  va_list ap;
  va_start(ap, format);
  auto l = vsnprintf(buf, 16348, format, ap);
  va_end(ap);
  if (conhost_) {
    return BaseWriteConhost(buf, l);
  }
  auto r = fwrite(buf, 1, l, stdout);
  fflush(stdout);
  return r;
}

#else
#include <unistd.h>

int BaseWriteTTY(bool bold, int color, const void *buf, size_t len) {
  if (bold) {
    fprintf(stderr, "\033[1;%dm", color);
  } else {
    fprintf(stderr, "\033[%dm", color);
  }
  auto l = fwrite(buf, 1, len, stderr);
  fwrite("\e[0m", 1, sizeof("\e[0m") - 1, stderr);
  return l;
}

int Printe(const char *format, ...) {
  char buf[8192];
  va_list ap;
  va_start(ap, format);
  auto l = vsnprintf(buf, 8192, format, ap);
  va_end(ap);
  if (isatty(STDERR_FILENO)) {
    return BaseWriteTTY(false, 31, buf, l);
  }
  return fwrite(buf, 1, l, stderr);
}
int Printw(const char *format, ...) {
  char buf[8192];
  va_list ap;
  va_start(ap, format);
  auto l = vsnprintf(buf, 8192, format, ap);
  va_end(ap);
  if (isatty(STDERR_FILENO)) {
    return BaseWriteTTY(true, 33, buf, l);
  }
  return fwrite(buf, 1, l, stderr);
}
int Print(const char *format, ...) {
  va_list ap;
  va_start(ap, format);
  auto l = vfprintf(stdout, format, ap);
  va_end(ap);
  return l;
}

#endif
