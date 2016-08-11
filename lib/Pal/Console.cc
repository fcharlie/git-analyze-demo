/*
* Console.cc
* git-analyze Pal
* author: Force.Charlie
* Date: 2016.08
* Copyright (C) 2016. OSChina.NET. All Rights Reserved.
*/
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>

#ifdef _WIN32
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
  SetConsoleTextAttribute(hConsole, FOREGROUND_INTENSITY | FOREGROUND_RED);
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

#else
#include <unistd.h>

int BaseErrorWriteTTY(const void *buf, size_t len) {
  // echo -e "\e[1;31m This is red text \e[0m"
  write(STDERR_FILENO, buf, len);
  return 0;
}

int BaseErrorMessageWrite(const char *format, va_list ap) {
  ////
  return 0;
}

int BaseErrorMessagePrint(const char *format, ...) {
  //
  return 0;
}

#endif
