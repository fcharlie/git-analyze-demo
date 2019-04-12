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
#include <argv.hpp>
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

Example:
  git-rollback --git-dir=/path/to/repo --backrev=7
  git-rollback --git-dir=/path/to/repo --backid=commitid

)";
  aze::FPrintF(stderr, "%s\n", ua);
}

bool parse_opts(int argc, char **argv, rollback_options &opt) {
  if (argc == 1) {
    usage();
    return false;
  }
  av::ParseArgv pa(argc, argv);
  pa.Add("git-dir", av::required_argument, 'g')
      .Add("backid", av::required_argument, 'I')
      .Add("backrev", av::required_argument, 'R')
      .Add("refname", av::required_argument, 'N')
      .Add("version", av::no_argument, 'v')
      .Add("verbose", av::no_argument, 'V')
      .Add("help", av::no_argument, 'h');
  av::error_code ec;
  auto result = pa.Execute(
      [&](int ch, const char *oa, const char *) {
        switch (ch) {
        case 'g':
          opt.gitdir = oa;
          break;
        case 'I':
          opt.oid = oa;
          break;
        case 'R': {
          int rev = 0;
          if (absl::SimpleAtoi(oa, &rev)) {
            opt.rev = rev;
          }
        } break;
        case 'N':
          opt.refname = oa;
          if (!aze::starts_with(opt.refname, "refs/heads/") &&
              opt.refname != "HEAD") {
            opt.refname = aze::strcat("refs/heads/", opt.refname);
          }
          break;
        case 'v':
          aze::FPrintF(stderr, "git-rollback 1.0\n");
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
      },
      ec);
  if (!result && ec.ec != av::SkipParse) {
    aze::FPrintF(stderr, "Parse argv error: %s\n", ec.message);
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
  rollback_options opt;
  if (!parse_opts(argc, argv, opt)) {
    return 1;
  }
  return ExecuteWithOptions(opt) ? 0 : 1;
}
