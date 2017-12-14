/*
* driver.cc
* git-analyze(git-author)
* author: Force.Charlie
* Date: 2017.06
* Copyright (C) 2017. GITEE.COM. All Rights Reserved.
*/
/// to create total year commits
#include <ctime>
#include <cstdio>
#include <cstring>
#include <string>
#include <git2.h>
#include <Pal.hpp>

//// this is real main
int Main(int argc, char **argv) {
  //
  return 0;
}

#if defined(_WIN32) && !defined(__CYGWIN__)
//// When use Visual C++, Support convert encoding to UTF8
#include <stdexcept>
#include <Windows.h>
//// To convert Utf8
char *CopyToUtf8(const wchar_t *wstr) {
  auto l = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, NULL, 0, NULL, NULL);
  char *buf = (char *)malloc(sizeof(char) * l + 1);
  if (buf == nullptr)
    throw std::runtime_error("Out of Memory ");
  WideCharToMultiByte(CP_UTF8, 0, wstr, -1, buf, l, NULL, NULL);
  return buf;
}
int wmain(int argc, wchar_t **argv) {
  std::vector<char *> Argv_;
  auto Release = [&]() {
    for (auto &a : Argv_) {
      free(a);
    }
  };
  try {
    for (int i = 0; i < argc; i++) {
      Argv_.push_back(CopyToUtf8(argv[i]));
    }
  } catch (const std::exception &e) {
    Printe("Exception: %s\n", e.what());
    Release();
    return -1;
  }
  auto result = Main(Argv_.data(), Argv_.size());
  Release();
  return result;
}
#else
int main(int argc, char **argv) {
  /* code */
  return Main(argc, argv);
}
#endif
