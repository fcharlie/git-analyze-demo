///  clang++ -std=c++17 argvex_test.cc -I../include
/// ./a.out -U=https://github.com --uri=https://wwww.baidu.com  -V
#include <cstring>
#include "argvex.hpp"

struct Dcontext {
  std::vector<std::string> urls;
  std::string out;
  int tries{5};
  int location{3};
  bool disposition{true};
  bool verbose{false};
};

template <typename Integer>
ax::ErrorResult Fromwchars(std::string_view sv_, Integer &iv) {
  return ax::Integer_from_chars(sv_, iv, 10);
}

ax::ErrorResult Fromwchars(std::string_view sv_, bool &bv) {
  if (strcasecmp(sv_.data(), "true") == 0 ||
      strcasecmp(sv_.data(), "on") == 0 || strcasecmp(sv_.data(), "yes") == 0 ||
      sv_.compare("1") == 0) {
    bv = true;
    return ax::ErrorResult{};
  }
  if (strcasecmp(sv_.data(), "false") == 0 ||
      strcasecmp(sv_.data(), "off") == 0 || strcasecmp(sv_.data(), "no") == 0 ||
      sv_.compare("0") == 0) {
    bv = false;
    return ax::ErrorResult{};
  }
  return ax::ErrorResult{std::string("Illegal boolean ").append(sv_), 1};
}

void PrintUsage() {
  const char *ua = R"(OVERVIEW: Noti windows download utils
Usage: noti [options] <input>
OPTIONS:
  -C [--content-disposition]       honor the Content-Disposition header when
                                     choosing local file names
  -h [--help]                      print noti usage information and exit
  -L [--location]                  location redirect level.
  -O [--output]                    save to path. single download task.
  -T [--tries]                     set number of retries to NUMBER (0 unlimits)
  -U [--uri]                       set download uri, default value.
  -v [--version]                   print noti version and exit
  -V [--verbose]                   print noti download verbose message
)";
  printf("%s", ua);
}

int ParseArgvImplement(int Argc, char **Argv, Dcontext &dctx) {
  ax::ParseArgv pa(Argc, Argv);
  std::vector<ax::ParseArgv::option> opts = {
      {"content-disposition", ax::ParseArgv::optional_argument,
       'C'}, /// --content-disposition=false -dfalse -dtrue -d=true -d=false
      {"help", ax::ParseArgv::no_argument, 'h'},
      {"location", ax::ParseArgv::required_argument, 'L'},
      {"uri", ax::ParseArgv::required_argument, 'U'},
      {"output", ax::ParseArgv::required_argument, 'O'},
      {"tries", ax::ParseArgv::required_argument, 'T'},
      {"version", ax::ParseArgv::no_argument, 'v'},
      {"verbose", ax::ParseArgv::no_argument, 'V'}};
  auto err =
      pa.ParseArgument(opts, [&](int ch, const char *optarg, const char *raw) {
        switch (ch) {
        case 'h':
          PrintUsage();
          exit(0);
        case 'C':
          if (optarg != nullptr) {
            auto err = Fromwchars(optarg, dctx.disposition);
            if (err.errorcode != 0) {
              return false;
            }
          }
          break;
        case 'L':
          if (optarg != nullptr) {
            auto err = Fromwchars(optarg, dctx.location);
            if (err.errorcode != 0) {
              return false;
            }
          }
          break;
        case 'U':
          if (optarg != nullptr) {
            dctx.urls.push_back(optarg);
          }
          break;
        case 'T':
          if (optarg != nullptr) {
            auto err = Fromwchars(optarg, dctx.tries);
            if (err.errorcode != 0) {
              return false;
            }
          }
          break;
        case 'O':
          if (optarg != nullptr) {
            dctx.out = optarg;
          }
          break;
        case 'v':
          printf("1.0.0\n");
          exit(0);
        case 'V':
          dctx.verbose = true;
          break;
        default:
          printf("Error Argument: %s\n", raw != nullptr ? raw : "unknown");
          return false;
        }
        return true;
      });
  if (err.errorcode != 0) {
    if (err.errorcode == 1) {
      printf("ParseArgv: %s\n", err.message.c_str());
    }
    return 1;
  }
  for (auto &v : pa.UnresolvedArgs()) {
    dctx.urls.push_back(v.data()); /// url append
  }
  return 0;
}
int main(int argc, char **argv) {
  /* code */
  Dcontext dctx;
  if (ParseArgvImplement(argc, argv, dctx) != 0) {
    return 1;
  }
  for (const auto &u : dctx.urls) {
    /* code */
    fprintf(stderr, "url: %s\n", u.c_str());
  }
  fprintf(stderr, "out: %s\n", dctx.out.c_str());
  if (dctx.verbose) {
    fprintf(stderr, "verbose model\n");
  }
  if (dctx.disposition) {
    fprintf(stderr, "disposition model\n");
  }
  fprintf(stderr, "location: %d\ntries: %d\n", dctx.location, dctx.tries);
  return 0;
}
