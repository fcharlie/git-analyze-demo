////No historical records branch
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <Pal.hpp>
#include <git2.h>

bool nullable_commit_create(git_repository *repo, const git_commit *commit,
                            const char *branch, const char *message) {
  git_oid newoid;
  auto author = git_commit_author(commit);
  auto committer = git_commit_committer(commit);
  git_tree *tree = nullptr;
  if (git_commit_tree(&tree, commit) != 0) {
    auto err = giterr_last();
    BaseErrorMessagePrint("git_commit_tree() %s\n", err->message);
    return false;
  }
  std::string refname = std::string("refs/heads/") + branch;
  if (git_commit_create(&newoid, repo, refname.c_str(), author, committer, NULL,
                        message, tree, 0, nullptr) != 0) {
    auto err = giterr_last();
    BaseErrorMessagePrint("git_commit_create() %s\n", err->message);
    git_tree_free(tree);
    return false;
  }
  BaseConsoleWrite("[%s %s]\ncommitter: %s\nemail: %s\nmessage: %s\n\n", branch,
                   git_oid_tostr_s(&newoid), committer->name, committer->email,
                   message);
  git_tree_free(tree);
  return true;
}

class LibgitHelper {
public:
  LibgitHelper() { git_libgit2_init(); }
  ~LibgitHelper() { git_libgit2_shutdown(); }
};

bool discover_commit(const char *gitdir, const char *branch,
                     const char *message) {
  static LibgitHelper hepler;
  git_repository *repo = nullptr;
  if (git_repository_open(&repo, gitdir) != 0) {
    BaseErrorMessagePrint("invaild git repository: %s\n", gitdir);
    return false;
  }
  git_reference *ref = nullptr;
  if (git_repository_head(&ref, repo) != 0) {
    auto err = giterr_last();
    BaseErrorMessagePrint("cannot open head %s\n", err->message);
    git_repository_free(repo);
    return false;
  }
  git_reference *xref{nullptr};
  if (git_reference_resolve(&xref, ref) != 0) {
    auto err = giterr_last();
    BaseErrorMessagePrint("Resolve reference: %s\n", err->message);
    git_reference_free(ref);
    git_repository_free(repo);
    return false;
  }
  auto oid = git_reference_target(xref);
  if (!oid) {
    auto err = giterr_last();
    BaseErrorMessagePrint("Lookup commit: %s\n", err->message);
    git_reference_free(xref);
    git_reference_free(ref);
    git_repository_free(repo);
    return false;
  }
  git_commit *commit = nullptr;
  if (git_commit_lookup(&commit, repo, oid) != 0) {
    auto err = giterr_last();
    BaseErrorMessagePrint("Lookup commit tree: %s\n", err->message);
    git_reference_free(xref);
    git_reference_free(ref);
    git_repository_free(repo);
    return false;
  }

  auto result = nullable_commit_create(repo, commit, branch, message);
  git_commit_free(commit);
  git_reference_free(xref);
  git_reference_free(ref);
  git_repository_free(repo);
  // git_reference_target(const git_reference *ref)
  return result;
}

#if defined(_WIN32) && !defined(__CYGWIN__)
//// When use Visual C++, Support convert encoding to UTF8
#include <stdexcept>
#include <Windows.h>
//// To convert Utf8
char *CopyToUtf8(const wchar_t *wstr) {
  auto l = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, NULL, 0, NULL, NULL);
  char *buf = (char *)malloc(sizeof(char) * l + 1);
  if (buf == nullptr)
    throw std::runtime_error("Out of Memory ");
  WideCharToMultiByte(CP_UTF8, 0, wstr, -1, buf, l, NULL, NULL);
  return buf;
}
int wmain(int argc, wchar_t **argv) {
  if (argc < 3) {
    BaseErrorMessagePrint("usage %s branch message\n", argv[0]);
    return 1;
  }
  std::vector<char *> Argv_;
  auto Release = [&]() {
    for (auto &a : Argv_) {
      free(a);
    }
  };
  try {
    for (int i = 0; i < argc; i++) {
      Argv_.push_back(CopyToUtf8(argv[i]));
    }
  } catch (const std::exception &e) {
    BaseErrorMessagePrint("Exception: %s\n", e.what());
    Release();
    return -1;
  }
  auto result = discover_commit(".", Argv_[1], Argv_[2]);
  Release();
  return result ? 0 : 1;
}
#else

int main(int argc, char **argv) {
  if (argc < 3) {
    BaseErrorMessagePrint("usage %s branch message\n", argv[0]);
    return 1;
  }
  if (!discover_commit(".", argv[1], argv[2]))
    return 1;
  return 0;
}
#endif
