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
#include <argvex.hpp>
#include <git.hpp>
#include <console.hpp>
#include <os.hpp>
#include "executor.hpp"

/*
 * git-analyze argument
 * git-analyze [<options>...] [--] [<pathspec>...] [<refs|branches>...]
 */

void usage() {
  const constexpr char *ua =
      R"(OVERVIEW: GIT analyze tools
 Usage: git-analyze <options>...] [--] [<pathspec>...] [<refs|branches> ...]
 OPTIONS:
   -h [--help]      print usage and exit
   --limitsize      set analyze engine limit blob size, example: 100MB
   --warnsize       set analyze engine warn blob size
   --timeout        set analyze engine lifycycle (ms)
   --who            show who is commit's author
   --all            analyze will scanf all refs
 )";
  aze::FPrintF(stdout, "%s\n", ua);
}
template <typename Integer>
ax::error_code Fromchars(std::string_view sv_, Integer &iv) {
  return ax::Integer_from_chars(sv_, iv, 10);
}
const constexpr std::uint64_t GB = 1024 * 1024 * 1024ull;
const constexpr std::uint64_t MB = 1024 * 1024ull;
ax::error_code Fromsize(std::string_view sv_, std::uint64_t &iv) {
  std::uint64_t size = 1;
  if (aze::ends_with(sv_, "MB") || aze::ends_with(sv_, "mb")) {
    size = MB;
    sv_.remove_suffix(2);
  } else if (aze::ends_with(sv_, "GB") || aze::ends_with(sv_, "gb")) {
    size = GB;
    sv_.remove_suffix(2);
  } else if (aze::ends_with(sv_, "KB") || aze::ends_with(sv_, "kb")) {
    size = 1024;
    sv_.remove_suffix(2);
  }
  std::uint64_t k;
  auto ec = ax::Integer_from_chars(sv_, k, 10);
  if (!ec) {
    iv = k * size;
  }
  return ec;
}

std::uint64_t AzeEnv(std::string_view key, std::uint64_t dv) {
  auto s = os::GetEnv(key);
  if (s.empty()) {
    return dv;
  }
  std::uint64_t xs;
  auto ec = Fromsize(s, xs);
  if (ec) {
    return dv;
  }
  return xs;
}

struct aze_options {
  std::string gitdir;  // gitdir pos 0
  std::string refname; // pos 1
  std::uint64_t largesize{0};
  std::uint64_t warnsize{0};
  std::int64_t timeout{-1}; // ms
  bool showcommitter{false};
  bool allrefs{false};
  bool verbose{false};
};

bool parse_opts(int argc, char **argv, aze_options &opt) {
  if (argc == 1) {
    usage();
    return false;
  }
  std::vector<ax::ParseArgv::option> opts = {
      {"git-dir", ax::ParseArgv::required_argument, 'g'},
      {"who", ax::ParseArgv::no_argument, 'w'},
      {"all", ax::ParseArgv::no_argument, 'A'},
      {"timeout", ax::ParseArgv::required_argument, 'T'},
      {"limitsize", ax::ParseArgv::required_argument, 'L'},
      {"warnsize", ax::ParseArgv::required_argument, 'W'},
      {"version", ax::ParseArgv::no_argument, 'v'},
      {"verbose", ax::ParseArgv::no_argument, 'V'},
      {"help", ax::ParseArgv::no_argument, 'h'}};
  ax::ParseArgv pa(argc, argv);
  auto ec =
      pa.Parse(opts, [&](int ch, const char *optarg, const char *) {
        switch (ch) {
        case 'g':
          opt.gitdir = optarg;
          break;
        case 'w':
          opt.showcommitter = true;
          break;
        case 'A':
          opt.allrefs = true;
          break;
        case 'L':
          Fromsize(optarg, opt.largesize);
          break;
        case 'W':
          Fromsize(optarg, opt.warnsize);
          break;
        case 'T':
          Fromchars(optarg, opt.timeout);
          break;
        case 'v':
          aze::FPrintF(stdout, "git-analyze 1.0\n");
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
  if (ec && ec.ec != ax::SkipParse) {
    aze::FPrintF(stderr, "%s\n", ec.message);
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
  if (opt.largesize == 0) {
    opt.largesize = AzeEnv("GIT_AZE_LIMITSIZE", 100 * MB);
  }
  if (opt.warnsize == 0) {
    opt.warnsize = AzeEnv("GIT_AZE_WARNSIZE", 50 * MB);
  }
  if (opt.gitdir.empty()) {
    opt.gitdir = ".";
  }
  if (!opt.allrefs && opt.refname.empty()) {
    opt.refname = "HEAD";
  };
  return true;
}

int cmd_main(int argc, char **argv) {
  aze_options opt;
  if (!parse_opts(argc, argv, opt)) {
    return 1;
  }
  git::global_initializer_t gi;
  aze::Executor e(opt.largesize, opt.warnsize); // to init
  fprintf(stderr,
          "git-analyze details\nlimit size:      %4.2f MB\nwarnning size:   "
          "%4.2f MB\n",
          ((double)opt.largesize / MB), ((double)opt.warnsize / MB));

  if (!e.Initialize(opt.gitdir)) {
    return 1;
  }
  if (opt.allrefs) {
    return e.AzeAll(opt.timeout) ? 0 : 1;
  }

  return e.AzeOne(opt.refname, opt.timeout) ? 0 : 1;
}
