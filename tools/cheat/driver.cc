////
#include <iomanip>
#include <sstream>
#include <string>
#include <argvex.hpp>
#include <console.hpp>
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

template <typename Integer>
ax::error_code Fromwchars(std::string_view sv_, Integer &iv) {
  return ax::Integer_from_chars(sv_, iv, 10);
}

bool cmd_options(int argc, char **argv, cheat_options &opt) {
  if (argc == 1) {
    usage();
    return false;
  }
  std::vector<ax::ParseArgv::option> opts = {
      {"git-dir", ax::ParseArgv::required_argument, 'g'},
      {"branch", ax::ParseArgv::required_argument, 'b'},
      {"message", ax::ParseArgv::required_argument, 'm'},
      {"parent", ax::ParseArgv::required_argument, 'p'},
      {"tree", ax::ParseArgv::required_argument, 't'},
      {"date", ax::ParseArgv::required_argument, 'd'},
      {"author", ax::ParseArgv::required_argument, 'a'},
      {"author-email", ax::ParseArgv::required_argument, 'e'},
      {"committer", ax::ParseArgv::required_argument, 'c'},
      {"committer-email", ax::ParseArgv::required_argument, 'E'},
      {"date", ax::ParseArgv::required_argument, 'd'},
      {"keep", ax::ParseArgv::no_argument, 'k'},
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
        case 'b':
          opt.branch = optarg;
          break;
        case 'm':
          opt.message = optarg;
          break;
        case 'p':
          opt.parent = optarg;
          break;
        case 't':
          opt.treedir = optarg;
          break;
        case 'a':
          opt.author = optarg;
          break;
        case 'c':
          opt.committer = optarg;
          break;
        case 'e':
          opt.authoremail = optarg;
          break;
        case 'E':
          opt.commiteremail = optarg;
          break;
        case 'k':
          opt.kauthor = true;
          break;
        case 'd': {
          struct std::tm tm;
          std::istringstream ss(optarg);
          ss >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%SZ");
#ifndef _WIN32
          opt.timeoff = tm.tm_gmtoff; // UNIX ONLY
#endif
          opt.date = mktime(&tm);
        } break;
        case 'v':
          printf("1.0\n");
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
    fprintf(stderr, "cheat execute failed\n");
    return 1;
  }
  return 0;
}
