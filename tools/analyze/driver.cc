/*
* driver.cc
* git-analyze
* author: Force.Charlie
* Date: 2016.08
* Copyright (C) 2016. OSChina.NET. All Rights Reserved.
*/
#include <cstdio>
#include <cstdlib>
#include <cstring>
#define ARGV_NO_LINK
#include <Argv.hpp>
#include "analyze.hpp"

/*
* git-analyze argument
* git-analyze [<options>...] [--] [<pathspec>...] [<refs|branch>...]
*/

void AnalyzeUsage() {
  ///
}

template <class T> inline void ResolveInteger(const char *cstr, T &t) {
  char *c;
  auto i = strtol(cstr, &c, 10);
  if (i > 0) {
    t = static_cast<T>(i);
  }
}

int ProcessArgv(int Argc, char **Argv, AnalyzeArgs &analyzeArgs) {
  int i = 1;
  for (; i < Argc; i++) {
    const char *arg = Argv[i];
    if (IsArg(arg, "--timeout")) {
      if (++i < Argc) {
        ResolveInteger(Argv[i], analyzeArgs.timeout);
      }
    } else if (IsArg(arg, "--limitsize")) {
      if (++i < Argc) {
        std::size_t limit_ = 0;
        ResolveInteger(Argv[i], limit_);
        analyzeArgs.limitsize = limit_ * MBSIZE;
      }
    } else if (IsArg(arg, "--warnsize")) {
      if (++i < Argc) {
        std::size_t warn_ = 0;
        ResolveInteger(Argv[i], warn_);
        analyzeArgs.warnsize = warn_ * MBSIZE;
      }
    } else if (IsArg(arg, "--all")) {
      analyzeArgs.allrefs = true;
    } else if (IsArg(arg, "-h", "--help")) {
      AnalyzeUsage();
      exit(0);
    } else {
      /// default break
      break;
    }
  }
  ///
  if (i <= Argc - 2) {
    analyzeArgs.repository.assign(Argv[i]);
    analyzeArgs.ref.assign(Argv[i + 1]);
  } else if (i <= Argc - 1) {
    analyzeArgs.repository.append(Argv[i]);
    analyzeArgs.ref.assign("HEAD");
  } else {
    ///
    analyzeArgs.repository.assign(".");
    analyzeArgs.ref.assign("HEAD");
  };

  //// To check default value
  if (analyzeArgs.timeout == -1) {
    analyzeArgs.timeout = EnvTimeout();
  }
  if (analyzeArgs.limitsize == 0) {
    analyzeArgs.limitsize = EnvLimitSize();
  }
  if (analyzeArgs.warnsize == 0) {
    analyzeArgs.warnsize = EnvWarnSize();
  }

  return 0;
}

void DebugAnalyzeArgs(const AnalyzeArgs &a) {
  printf("%s %s\nLimit %ld Warn %ld Timeout %ld All Switch %d\n",
         a.repository.c_str(), a.ref.c_str(), a.limitsize, a.warnsize,
         a.timeout, a.allrefs);
}

int main(int argc, char **argv) {
  AnalyzeArgs analyzeArgs;
  ProcessArgv(argc, argv, analyzeArgs);
  DebugAnalyzeArgs(analyzeArgs);
  return 0;
}
