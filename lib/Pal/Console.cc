/*
* Console.cc
* git-analyze Pal
* author: Force.Charlie
* Date: 2016.08
* Copyright (C) 2016. OSChina.NET. All Rights Reserved.
*/
#include <stdio.h>
#include <stdlib.h>
#ifdef _WIN32
#include <Windows.h>
#include <io.h>
/// check Stdhandle
bool IsUnderConsoleHost() {
  // HANDLE hStderr = fdtoh(STDERR_FILENO);
  //_isatty()
  // return (GetFileType(hStderr) == FILE_TYPE_CHAR);
  return _isatty(_fileno(stderr)) != 0;
}

int BaseErrorWriteConsoleHost(const char *buf, size_t len) {
  //
  return 0;
}

int BaseErrorMessagePrint(const char *format, ...) {
  //
  return 0;
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
