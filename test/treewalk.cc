//// Tree walk test
#include <cstdio>
#include <git2.h>
#include <stdexcept>

class RaiiRepository {
public:
  RaiiRepository(const char *repodir, const char *branch)
      : repo_(nullptr), commit_(nullptr) {
    ///
    if (git_repository_open(&repo_, repodir) != 0) {
      auto err = giterr_last();
      throw std::runtime_error(err->message);
    }
    git_reference *ref_{nullptr};
    if (git_reference_lookup(&ref_, repo_, branch) != 0) {
      //// second look branch to ref
      if (git_branch_lookup(&ref_, repo_, branch, GIT_BRANCH_LOCAL) != 0) {
        auto err = giterr_last();
        throw std::runtime_error(err->message);
      }
    }
    auto id = git_reference_target(ref_);
    if (git_commit_lookup(&commit_, repo_, id) != 0) {
      auto err = giterr_last();
      git_reference_free(ref_);
      throw std::runtime_error(err->message);
    }
    /////
  }
  ~RaiiRepository() {
    if (commit_) {
      git_commit_free(commit_);
    }
    if (repo_) {
      git_repository_free(repo_);
    }
  }
  bool Treewarlk();
  git_commit *commit() { return commit_; }
  git_repository *repository() { return repo_; }

private:
  git_repository *repo_;
  git_commit *commit_;
};

int git_treewalk_resolveblobs(const char *root, const git_tree_entry *entry,
                              void *payload) {
  //
  if (git_tree_entry_type(entry) == GIT_OBJ_BLOB) {
    fprintf(stderr, "%s%s\n", root, git_tree_entry_name(entry));
  }
  return 0;
}

bool RaiiRepository::Treewarlk() {
  git_tree *tree{nullptr};
  if (git_commit_tree(&tree, commit_) != 0) {
    return false;
  }
  git_tree_walk(tree, GIT_TREEWALK_PRE, git_treewalk_resolveblobs, this);
  git_tree_free(tree);
  return true;
}

bool treewalk(const char *repodir, const char *branch) {
  //
  try {
    RaiiRepository repo(repodir, branch);
    repo.Treewarlk();
  } catch (const std::exception &e) {
    fprintf(stderr, "Exception:%s\n", e.what());
    return false;
  }
  return true;
}

int main(int argc, char const *argv[]) {
  /* code */
  if (argc < 3) {
    fprintf(stderr, "usage %s repodir branch\n", argv[0]);
    return 1;
  }
  git_libgit2_init();
  if (!treewalk(argv[1], argv[2])) {
    fprintf(stderr, "Parse failed \n");
  } else {
    fprintf(stderr, "Parse completed\n");
  }
  git_libgit2_shutdown();
  return 0;
}
