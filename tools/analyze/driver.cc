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
#include <vector>
#define ARGV_NO_LINK
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
  --all            analyze will scanf all refs
)";
  printf("%s\n", kUsage);
}

template <class T> inline void ResolveInteger(const char *cstr, T &t) {
  char *c;
  auto i = strtol(cstr, &c, 10);
  if (i > 0) {
    t = static_cast<T>(i);
  }
}

int ProcessArgv(int Argc, char **Argv, AnalyzeArgs &analyzeArgs) {
  std::vector<const char *> Args;
  for (int i = 1; i < Argc; i++) {
    const char *arg = Argv[i];
    if (IsArg(arg, "--timeout")) {
      if (++i < Argc) {
        ResolveInteger(Argv[i], analyzeArgs.timeout);
      }
    } else if (IsArg(arg, "--limitsize")) {
      if (++i < Argc) {
        std::size_t limit_ = 0;
        ResolveInteger(Argv[i], limit_);
        g_limitsize = limit_ * MBSIZE;
      }
    } else if (IsArg(arg, "--warnsize")) {
      if (++i < Argc) {
        std::size_t warn_ = 0;
        ResolveInteger(Argv[i], warn_);
        g_warnsize = warn_ * MBSIZE;
      }
    } else if (IsArg(arg, "--all")) {
      analyzeArgs.allrefs = true;
    } else if (IsArg(arg, "-h", "--help")) {
      AnalyzeUsage();
      exit(0);
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

  //// To check default value
  if (analyzeArgs.timeout == -1) {
    analyzeArgs.timeout = EnvTimeout();
  }
  if (g_limitsize == 0) {
    g_limitsize = EnvLimitSize();
  }
  if (g_warnsize == 0) {
    g_warnsize = EnvWarnSize();
  }

  return 0;
}

int main(int argc, char **argv) {
  AnalyzeArgs analyzeArgs;
  ProcessArgv(argc, argv, analyzeArgs);
  if (ProcessAnalyzeTask(analyzeArgs)) {
    printf("git-analyze: Operation completed !\n");
  } else {
    fprintf(stderr, "git-analyze: Operation aborted !\n");
  }
  return 0;
}
