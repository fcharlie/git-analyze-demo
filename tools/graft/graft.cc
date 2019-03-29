/// THIS IS GRAFT COMMAND
#include <cstdio>
#include <cstdlib>
#include <string>
#include <string_view>
#include <argvex.hpp>
#include <git.hpp>
#include <optional>

struct graft_info_t {
  std::string gitdir{"."};
  std::string branch;
  std::string commitid;
  std::string message;
};

void PrintUsage() {
  const char *ua = R"(OVERVIEW: git-graft
Usage: git-graft [options] <input>
OPTIONS:
  -h [--help]                      print git-graft usage information and exit
  -b [--branch]                    new branch name.
  -d [--git-dir]                   repository path.
  -m [--message]                   commit message
Example:
  git-graft commit-id -m "message"
)";
  printf("%s", ua);
}

bool parse_argv(int argc, char **argv, graft_info_t &gf) {
  std::vector<ax::ParseArgv::option> opts = {
      {"help", ax::ParseArgv::no_argument, 'h'},
      {"git-dir", ax::ParseArgv::required_argument, 'd'},
      {"message", ax::ParseArgv::required_argument, 'm'},
      {"branch", ax::ParseArgv::required_argument, 'b'}
      //
  };
  ax::ParseArgv pv(argc, argv);
  auto err =
      pv.ParseArgument(opts, [&](int ch, const char *optarg, const char *raw) {
        switch (ch) {
        case 'h':
          PrintUsage();
          exit(0);
          break;
        case 'b':
          gf.branch = optarg;
          break;
        case 'm':
          gf.message = optarg;
          break;
        case 'd':
          gf.gitdir = optarg;
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
    return false;
  }
  if (pv.UnresolvedArgs().size() < 1) {
    PrintUsage();
    return false;
  }
  gf.commitid = pv.UnresolvedArgs()[0];
  return true;
}

inline void SignatureCommiterFill(git_signature *sig,
                                  const git_signature *old) {
  sig->when = old->when;
  auto name = getenv("GIT_COMMITTER_NAME");
  if (name != nullptr) {
    sig->name = name;
  } else {
    sig->name = old->name;
  }
  auto email = getenv("GIT_COMMITTER_EMAIL");
  if (email != nullptr) {
    sig->email = email;
  } else {
    sig->email = old->email;
  }
}
void SignatureAuthorFill(git_signature *sig, const git_signature *old) {
  sig->when = old->when;
  auto name = getenv("GIT_AUTHOR_NAME");
  if (name != nullptr) {
    sig->name = name;
  } else {
    sig->name = old->name;
  }
  auto email = getenv("GIT_AUTHOR_NAME");
  if (email != nullptr) {
    sig->email = email;
  } else {
    sig->email = old->email;
  }
}

void dump_error() {
  auto ec = giterr_last();
  if (ec != nullptr) {
    fprintf(stderr, "%s\n", ec->message);
  }
}

std::optional<std::string> make_refname(git::repository &r,
                                        std::string_view sv) {
  if (!sv.empty()) {
    return std::make_optional(aze::strcat("refs/heads/", sv));
  }
  auto ref = r.get_reference("HEAD");
  if (!ref) {
    return std::nullopt;
  }
  auto dr = ref->symbolic_target();
  if (!dr) {
    return std::nullopt;
  }
  auto p = git_reference_name(dr->p());
  if (p != nullptr) {
    return std::make_optional(p);
  }
  return std::nullopt;
}

bool graft_commit(const graft_info_t &gf) {
  git::error_code ec;
  auto repo = git::repository::make_repository(gf.gitdir, ec);
  if (!repo) {
    fprintf(stderr, "Error: %s\n", ec.message.data());
    return false;
  }
  auto commit = repo->get_commit(gf.commitid);
  if (!commit) {
    fprintf(stderr, "open commit: %s ", gf.commitid.c_str());
    dump_error();
    return false;
  }

  auto refname = make_refname(*repo, gf.branch);
  if (!refname) {
    fprintf(stderr, "unable lookup branch name\n");
    return false;
  }
  auto parent = repo->get_reference_commit(*refname);
  if (!parent) {
    fprintf(stderr, "open par commit: %s ", gf.branch.c_str());
    dump_error();
    return false;
  }
  ///

  git_tree *tree = nullptr;
  if (git_commit_tree(&tree, commit->p()) != 0) {
    dump_error();
    return false;
  }
  git_signature author, committer;
  SignatureAuthorFill(&author, git_commit_author(commit->p()));
  SignatureCommiterFill(&committer, git_commit_committer(commit->p()));
  std::string msg =
      gf.message.empty() ? git_commit_message(commit->p()) : gf.message;
  const git_commit *parents[] = {parent->p(), commit->p()};
  fprintf(stderr, "New commit, message: '%s'\n", msg.c_str());
  git_oid oid;
  if (git_commit_create(&oid, repo->p(), refname->c_str(), &author, &committer,
                        nullptr, msg.c_str(), tree, 2, parents) != 0) {
    dump_error();
    git_tree_free(tree);
    return false;
  }
  fprintf(stderr, "New commit id: %s\n", git_oid_tostr_s(&oid));
  git_tree_free(tree);
  return true;
}

int cmd_main(int argc, char **argv) {
  git::global_initializer_t gi;
  graft_info_t gf;
  if (!parse_argv(argc, argv, gf)) {
    return 1;
  }
  if (!graft_commit(gf)) {
    return 1;
  }
  return 0;
}
