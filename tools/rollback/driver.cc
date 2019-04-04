/*
 * driver.cc
 * git-rollback
 * author: Force.Charlie
 * Date: 2016.08
 * Copyright (C) 2019. GITEE.COM. All Rights Reserved.
 */
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <vector>
#include <console.hpp>
#include <argvex.hpp>
#include <git.hpp>
#include <string.hpp>
#include "rollback.hpp"

/*
 * git-rollback --git-dir=/path/to/repo --backrev=7
 * git-rollback --git-dir /path/to/repo --backrev 7
 * git-rollback --git-dir=/path/to/repo --backid=a59a9bb06a
 */

void usage() {
  const char *ua =
      R"(OVERVIEW: GIT rollback tools
Usage: git-rollback <options>...] [--] [<pathspec>...] [<refs|branches> ...]
OPTIONS:
  -h [--help]      print usage and exit
  --git-dir        set rollback repository path
  --backid         set rollback commit id
  --backrev        set rollback current back X rev
  --refname        set rollback current reference name
  --force          force gc prune

Example:
  git-rollback --git-dir=/path/to/repo --backrev=7
  git-rollback --git-dir=/path/to/repo --backid=commitid

)";
  fprintf(stderr, "%s\n", ua);
}

template <typename Integer>
ax::ErrorResult Fromchars(std::string_view sv_, Integer &iv) {
  return ax::Integer_from_chars(sv_, iv, 10);
}

bool parse_opts(int argc, char **argv, rollback_options &opt) {
  if (argc == 1) {
    usage();
    return false;
  }
  std::vector<ax::ParseArgv::option> opts = {
      {"git-dir", ax::ParseArgv::required_argument, 'g'},
      {"backid", ax::ParseArgv::required_argument, 'I'},
      {"backrev", ax::ParseArgv::required_argument, 'R'},
      {"refname", ax::ParseArgv::required_argument, 'N'},
      {"force", ax::ParseArgv::no_argument, 'F'},
      {"version", ax::ParseArgv::no_argument, 'v'},
      {"verbose", ax::ParseArgv::no_argument, 'V'},
      {"help", ax::ParseArgv::no_argument, 'h'}};
  ax::ParseArgv pa(argc, argv);
  auto err =
      pa.ParseArgument(opts, [&](int ch, const char *optarg, const char *) {
        switch (ch) {
        case 'g':
          opt.gitdir = optarg;
          break;
        case 'I':
          opt.oid = optarg;
          break;
        case 'R':
          Fromchars(optarg, opt.rev);
          break;
        case 'N':
          opt.refname = optarg;
          if (!aze::starts_with(opt.refname, "refs/heads/") &&
              opt.refname != "HEAD") {
            opt.refname = aze::strcat("refs/heads/", opt.refname);
          }
          break;
        case 'F':
          opt.forced = true;
          break;
        case 'v':
          printf("git-analyze 1.0\n");
          exit(0);
          break;
        case 'V':
          opt.verbose = true;
          break;
        case 'h':
          usage();
          exit(0);
          break;
        default:
          break;
        }
        return true;
      });
  if (!err) {
    aze::FPrintF(stderr, "Parse argv error: %s\n", err.message);
    return false;
  }

  /// Apply unresolved args to gitdir repo, and other
  size_t index = 0;
  auto sz = pa.UnresolvedArgs().size();
  if (opt.gitdir.empty() && index < sz) {
    opt.gitdir = pa.UnresolvedArgs()[index];
    index++;
  }
  if (index < sz) {
    opt.refname = pa.UnresolvedArgs()[index];
  }
  if (opt.gitdir.empty()) {
    opt.gitdir = ".";
  }
  if (opt.refname.empty()) {
    opt.refname = "HEAD";
  };
  return true;
  return 0;
}

int cmd_main(int argc, char **argv) {
  git::global_initializer_t gi;
  Executor e;
  if (!parse_opts(argc, argv, e.options())) {
    return 1;
  }
  return 0;
}
