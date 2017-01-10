/*
* driver.cc
* git-analyze
* author: Force.Charlie
* Date: 2016.08
* Copyright (C) 2017. OSChina.NET. All Rights Reserved.
*/
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <Pal.hpp>
#include <Argv.hpp>
#include "analyze.hpp"

/*
* git-analyze argument
* git-analyze [<options>...] [--] [<pathspec>...] [<refs|branches>...]
*/

void AnalyzeUsage() {
  const char *kUsage =
      R"(OVERVIEW: GIT analyze tools
Usage: git-analyze <options>...] [--] [<pathspec>...] [<refs|branches> ...]
OPTIONS:
  -h [--help]      print usage and exit
  --limitsize      set analyze engine limit blob size
  --warnsize       set analyze engine warn blob size
  --timeout        set analyze engine lifycycle
  --who            show who is commit's author
  --all            analyze will scanf all refs
)";
  printf("%s\n", kUsage);
}

template <class T> inline void ResolveInteger(const char *cstr, T &t) {
  char *c;
  auto i = std::strtol(cstr, &c, 10);
  if (i > 0) {
    t = static_cast<T>(i);
  }
}

int ProcessArgv(int Argc, char **Argv, AnalyzeArgs &analyzeArgs) {
  std::vector<const char *> Args;
  const char *va{nullptr};
  for (int i = 1; i < Argc; i++) {
    const char *arg = Argv[i];
    if (IsArg(arg, "--timeout", sizeof("--timeout") - 1, &va)) {
      if (va) {
        ResolveInteger(va, analyzeArgs.timeout);
      } else if (++i < Argc) {
        ResolveInteger(Argv[i], analyzeArgs.timeout);
      }
    } else if (IsArg(arg, "--limitsize", sizeof("--limitsize") - 1, &va)) {
      if (va) {
        std::size_t limit_ = 0;
        ResolveInteger(va, limit_);
        g_limitsize = limit_ * MBSIZE;
      } else if (++i < Argc) {
        std::size_t limit_ = 0;
        ResolveInteger(Argv[i], limit_);
        g_limitsize = limit_ * MBSIZE;
      }
    } else if (IsArg(arg, "--warnsize", sizeof("--warnsize") - 1, &va)) {
      if (va) {
        std::size_t warn_ = 0;
        ResolveInteger(va, warn_);
        g_warnsize = warn_ * MBSIZE;
      } else if (++i < Argc) {
        std::size_t warn_ = 0;
        ResolveInteger(Argv[i], warn_);
        g_warnsize = warn_ * MBSIZE;
      }
    } else if (IsArg(arg, "--all")) {
      analyzeArgs.allrefs = true;
    } else if (IsArg(arg, "--who")) {
      ////
      g_showcommitter = true;
    } else if (IsArg(arg, "-h", "--help")) {
      AnalyzeUsage();
      std::exit(0);
    } else {
      Args.push_back(arg);
    }
  }
  ///
  if (Args.size() >= 2) {
    analyzeArgs.repository.assign(Args[0]);
    analyzeArgs.ref.assign(Args[1]);
  } else if (Args.size() >= 1) {
    analyzeArgs.repository.append(Args[0]);
    analyzeArgs.ref.assign("HEAD");
  } else {
    ///
    analyzeArgs.repository.assign(".");
    analyzeArgs.ref.assign("HEAD");
  };
  PalEnvironment env;
  //// To check default value
  if (analyzeArgs.timeout == -1) {
    analyzeArgs.timeout = env.Integer(GIT_ANALYZE_TIMEOUT, -1);
  }
  if (g_limitsize == 0) {
    g_limitsize = env.Integer(GIT_ANALYZE_LIMITSIZE, 100) * MBSIZE;
  }
  if (g_warnsize == 0) {
    g_warnsize = env.Integer(GIT_ANALYZE_WARNSIZE, 50) * MBSIZE;
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
    BaseErrorMessagePrint("Exception: %s\n", e.what());
    Release();
    return -1;
  }
  AnalyzeArgs analyzeArgs;
  ProcessArgv((int)Argv_.size(), Argv_.data(), analyzeArgs);
  if (ProcessAnalyzeTask(analyzeArgs)) {
    BaseConsoleWrite("git-analyze: Operation completed !\n");
  } else {
    BaseErrorMessagePrint("git-analyze: Operation aborted !\n");
  }
  Release();
  return 0;
}
#else

int main(int argc, char **argv) {
  AnalyzeArgs analyzeArgs;
  ProcessArgv(argc, argv, analyzeArgs);
  if (ProcessAnalyzeTask(analyzeArgs)) {
    BaseConsoleWrite("git-analyze: Operation completed !\n");
  } else {
    BaseErrorMessagePrint("git-analyze: Operation aborted !\n");
  }
  return 0;
}
#endif
