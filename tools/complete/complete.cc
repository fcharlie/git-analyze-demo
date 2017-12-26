/*
* complete.cc
* git-analyze
* author: Force.Charlie
* Date: 2016.12
* Copyright (C) 2018. GITEE.COM. All Rights Reserved.
*/
/// to create total year commits
#include "complete.hpp"

//// this is real main
int Main(int argc, char **argv) {
  int start_year = 0;
  int end_year = 0;
  if (argc < 3) {
    Printe("usage: %s  dir branch message year(or year range) day\nExample: "
           "git-complete . dev 'no commit message' 2017~2020 \n",
           argv[0]);
    return 1;
  }
  Demolisher demolisher;
  if (!demolisher.Initialize(argv[1], argv[2], argv[3]))
    return 1;
  if (argc > 4) {
    char *c = nullptr;
    start_year = strtol(argv[4], &c, 10);
    if (c != nullptr && (*c == '~' || *c == '-')) {
      char *c2 = nullptr;
      end_year = strtol(c + 1, &c2, 10);
    }
    if (end_year == 0) {
      end_year = start_year;
    }
  }
  bool createNewbranch = false;
  if (argc > 5) {
    createNewbranch = (strcmp("--nb", argv[5]) == 0 || strcmp("--NB", argv[5]));
  }
  if (demolisher.IntervalFill(start_year, end_year, createNewbranch)) {
    Print("Has completed in %d to %d's commits !\n", start_year, end_year);
  }
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
