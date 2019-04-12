////
#include <iomanip>
#include <sstream>
#include <string>
#include <argv.hpp>
#include <console.hpp>
#include <absl/time/time.h>
#include "cheat.hpp"

void usage() {
  const char *ua = R"(OVERVIEW: Make a special git branch: git-cheat
Usage: git-cheat [options] <input>
OPTIONS:
  -g,--git-dir          Git repository dir,default open pwd, pos 0
  -b,--branch           New branch name, pos 1
  -m,--message          New commit message, pos 2
  -p,--parent           The new branch tree is based on which commit/branch.
  -t,--tree             Subtree or current commit root tree.
  -d,--date             New commit time.
  -a,--author           New commit author, default use old author.
  -c,--committer        New commit committer, default use old author.
  -e,--author-email     Author email.
  -E,--committer-email  Committer email.

SWITCH:
  -k,--keep             Keep commit signature.
  -v,--version          Print version information and exit.
  -V,--verbose          Print verbose message.
  -h,--help             Print help information and exit.

example:
  git-cheat . tool "new tools branch" --tree tools #new branch tool base subtree tools.
)";
  printf("%s\n", ua);
}

bool cmd_options(int argc, char **argv, cheat_options &opt) {
  if (argc == 1) {
    usage();
    return false;
  }
  av::ParseArgv pa(argc, argv);
  pa.Add("git-dir", av::required_argument, 'g')
      .Add("branch", av::required_argument, 'b')
      .Add("message", av::required_argument, 'm')
      .Add("parent", av::required_argument, 'p')
      .Add("tree", av::required_argument, 't')
      .Add("date", av::required_argument, 'd')
      .Add("author", av::required_argument, 'a')
      .Add("author", av::required_argument, 'a')
      .Add("author-email", av::required_argument, 'e')
      .Add("committer", av::required_argument, 'c')
      .Add("committer-email", av::required_argument, 'E')
      .Add("date", av::required_argument, 'd')
      .Add("keep", av::no_argument, 'k')
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
        case 'b':
          opt.branch = oa;
          break;
        case 'm':
          opt.message = oa;
          break;
        case 'p':
          opt.parent = oa;
          break;
        case 't':
          opt.treedir = oa;
          break;
        case 'a':
          opt.author = oa;
          break;
        case 'c':
          opt.committer = oa;
          break;
        case 'e':
          opt.authoremail = oa;
          break;
        case 'E':
          opt.commiteremail = oa;
          break;
        case 'k':
          opt.kauthor = true;
          break;
        case 'd': {
          absl::Time t;
          std::string err;
          std::string fmt = "%Y-%m-%dT%H:%M:%SZ";
          if (!absl::ParseTime(fmt, oa, &t, &err)) {
            absl::FPrintF(stderr, "Pase date: %s error: %s; we support: %s\n",
                          oa, err, fmt);
            return false;
          }
          auto tm = absl::ToTM(t, absl::LocalTimeZone());
#ifndef _WIN32
          opt.timeoff = tm.tm_gmtoff; // UNIX ONLY
#endif
          opt.date = mktime(&tm);
        } break;
        case 'v':
          aze::FPrintF(stderr, "git-cheat 1.0\n");
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
  if (!result) {
    if (ec.ec != av::SkipParse) {
      aze::FPrintF(stderr, "ParseArgv: %s\n", ec.message);
    }
    return false;
  }

  /// Apply unresolved args to gitdir repo, and other
  size_t index = 0;
  auto sz = pa.UnresolvedArgs().size();
  if (opt.gitdir.empty() && index < sz) {
    opt.gitdir = pa.UnresolvedArgs()[index].data();
    index++;
  }
  if (opt.branch.empty() && index < sz) {
    opt.branch = pa.UnresolvedArgs()[index].data();
    index++;
  }
  if (opt.message.empty() && index < sz) {
    opt.message = pa.UnresolvedArgs()[index].data();
    index++;
  }
  return true;
}

int cmd_main(int argc, char **argv) {
  cheat_options opt;
  if (!cmd_options(argc, argv, opt)) {
    return 1;
  }
  if (!cheat_execute(opt)) {
    aze::FPrintF(stderr, "cheat execute failed\n");
    return 1;
  }
  return 0;
}
