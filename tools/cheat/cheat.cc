////No historical records branch
#include <stdio.h>
#include <stdlib.h>
#include <git2.h>
#include <string>

bool nullable_commit_create(git_repository *repo, const git_commit *commit,
                            const char *ref, const char *message) {
  git_oid newoid;
  auto author = git_commit_author(commit);
  auto committer = git_commit_committer(commit);
  git_tree *tree = nullptr;
  if (git_commit_tree(&tree, commit) != 0) {
    auto err = giterr_last();
    fprintf(stderr, "git_commit_tree() %s\n", err->message);
    return false;
  }
  if (git_commit_create(&newoid, repo, ref, author, committer, NULL, message,
                        tree, 0, nullptr) != 0) {
    auto err = giterr_last();
    fprintf(stderr, "git_commit_create() %s\n", err->message);
    git_tree_free(tree);
    return false;
  }
  fprintf(stderr, "create branch: %s commit: %s\n", ref,
          git_oid_tostr_s(&newoid));
  /// git_commit_free(commit);
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
    fprintf(stderr, "invaild git repository: %s\n", gitdir);
    return false;
  }
  git_reference *ref = nullptr;
  if (git_repository_head(&ref, repo) != 0) {
    fprintf(stderr, "cannot open head\n");
    auto err = giterr_last();
    fprintf(stderr, "%s\n", err->message);
    git_repository_free(repo);
    return false;
  }
  git_reference *xref{nullptr};
  if (git_reference_resolve(&xref, ref) != 0) {
    auto err = giterr_last();
    fprintf(stderr, "%s\n", err->message);
    git_reference_free(ref);
    git_repository_free(repo);
    return false;
  }
  auto oid = git_reference_target(xref);
  if (!oid) {
    auto err = giterr_last();
    fprintf(stderr, "%s\n", err->message);
    git_reference_free(xref);
    git_reference_free(ref);
    git_repository_free(repo);
    return false;
  }
  git_commit *commit = nullptr;
  if (git_commit_lookup(&commit, repo, oid) != 0) {
    auto err = giterr_last();
    fprintf(stderr, "%s\n", err->message);
    git_reference_free(xref);
    git_reference_free(ref);
    git_repository_free(repo);
    return false;
  }
  std::string refname = std::string("refs/heads/") + branch;
  auto result = nullable_commit_create(repo, commit, refname.c_str(), message);
  git_commit_free(commit);
  git_reference_free(xref);
  git_reference_free(ref);
  git_repository_free(repo);
  // git_reference_target(const git_reference *ref)
  return result;
}

int main(int argc, char const *argv[]) {
  if (argc < 3) {
    fprintf(stderr, "usage %s branch message\n", argv[0]);
    return 1;
  }
  if (!discover_commit(".", argv[1], argv[2]))
    return 1;
  return 0;
}
