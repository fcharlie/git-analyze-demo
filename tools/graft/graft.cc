/// THIS IS GRAFT COMMAND
#include <cstdio>
#include <cstdlib>
#include <string>
#include <string_view>
#include <argv.hpp>
#include <git.hpp>
#include <optional>
#include <console.hpp>
#include <os.hpp>

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
  av::ParseArgv pv(argc, argv);

  pv.Add("help", av::no_argument, 'h')
      .Add("git-dir", av::required_argument, 'd')
      .Add("message", av::required_argument, 'm')
      .Add("branch", av::required_argument, 'b');
  av::error_code ec;
  auto result = pv.Execute(
      [&](int ch, const char *optarg, const char *raw) {
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
      },
      ec);
  if (!result && ec.ec != av::SkipParse) {
    aze::FPrintF(stderr, "ParseArgv: %s\n", ec.message);
    return false;
  }
  if (pv.UnresolvedArgs().size() < 1) {
    PrintUsage();
    return false;
  }
  gf.commitid = pv.UnresolvedArgs()[0];
  return true;
}

class SignatureSaver {
public:
  SignatureSaver() = default;
  SignatureSaver(const SignatureSaver &) = delete;
  SignatureSaver &operator=(const SignatureSaver &) = delete;
  void InitializeEnv() {
    name = os::GetEnv("GIT_COMMITTER_NAME");
    email = os::GetEnv("GIT_COMMITTER_EMAIL");
    aname = os::GetEnv("GIT_AUTHOR_NAME");
    aemail = os::GetEnv("GIT_AUTHOR_NAME");
  }

  void UpdateCommiter(git_signature *sig, const git_signature *old) {
    sig->email = email.empty() ? old->email : email.data();
    sig->name = name.empty() ? old->name : name.data();
    sig->when = old->when;
  }
  void UpdateAuthor(git_signature *sig, const git_signature *old) {
    sig->email = aemail.empty() ? old->email : aemail.data();
    sig->name = aname.empty() ? old->name : aname.data();
    sig->when = old->when;
  }

private:
  std::string name;
  std::string email;
  std::string aname;
  std::string aemail;
};

void dump_error() {
  auto ec = giterr_last();
  if (ec != nullptr) {
    aze::FPrintF(stderr, "%s\n", ec->message);
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
  auto repo = git::repository::make_repository_ex(gf.gitdir, ec);
  if (!repo) {
    aze::FPrintF(stderr, "Error: %s\n", ec.message);
    return false;
  }
  auto commit = repo->get_commit(gf.commitid);
  if (!commit) {
    aze::FPrintF(stderr, "open commit: %s ", gf.commitid);
    dump_error();
    return false;
  }

  auto refname = make_refname(*repo, gf.branch);
  if (!refname) {
    aze::FPrintF(stderr, "unable lookup branch name\n");
    return false;
  }
  auto parent = repo->get_reference_commit(*refname);
  if (!parent) {
    aze::FPrintF(stderr, "open par commit: %s ", gf.branch);
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
  SignatureSaver saver;
  saver.InitializeEnv();
  saver.UpdateAuthor(&author, git_commit_author(commit->p()));
  saver.UpdateCommiter(&committer, git_commit_committer(commit->p()));
  std::string msg =
      gf.message.empty() ? git_commit_message(commit->p()) : gf.message;
  const git_commit *parents[] = {parent->p(), commit->p()};
  aze::FPrintF(stderr, "New commit, message: '%s'\n", msg);
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
