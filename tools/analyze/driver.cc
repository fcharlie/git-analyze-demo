/*
 * driver.cc
 * git-analyze
 * author: Force.Charlie
 * Date: 2016.08
 * Copyright (C) 2019. GITEE.COM. All Rights Reserved.
 */
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <Argv.hpp>
#include <Pal.hpp>
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

int cmd_main(int argc, char **argv) {
  AnalyzeArgs analyzeArgs;
  ProcessArgv(argc, argv, analyzeArgs);
  if (ProcessAnalyzeTask(analyzeArgs)) {
    Print("git-analyze: Operation completed !\n");
  } else {
    Printe("git-analyze: Operation aborted !\n");
  }
  return 0;
}
