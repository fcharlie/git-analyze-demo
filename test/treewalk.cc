////////////////
#include <cstdio>
#include <string>
#include <string_view>
#include <git2.h>

struct treebase_t {
  ~treebase_t() {
    if (tree != nullptr) {
      git_tree_free(tree);
    }
    if (commit != nullptr) {
      git_commit_free(commit);
    }
  }
  git_commit *commit{nullptr};
  git_tree *tree{nullptr};

  git_commit *fromoid(git_repository *repo, std::string_view rev) {
    git_commit *c = nullptr;
    git_oid oid;
    if (git_oid_fromstrn(&oid, rev.data(), rev.size()) != 0) {
      return c;
    }
    if (git_commit_lookup(&c, repo, &oid) != 0) {
      return c;
    }
    return c;
  }

  git_commit *frombranch(git_repository *repo, std::string_view branch) {
    git_commit *c = nullptr;
    git_reference *ref_{nullptr};
    if (git_reference_lookup(&ref_, repo, branch.data()) != 0) {
      //// second look branch to ref
      if (git_branch_lookup(&ref_, repo, branch.data(), GIT_BRANCH_LOCAL) !=
          0) {
        return nullptr;
      }
    }
    auto id = git_reference_target(ref_);
    if (git_commit_lookup(&c, repo, id) != 0) {
      git_reference_free(ref_);
      return nullptr;
    }
    git_reference_free(ref_);
    return c;
  }

  bool lookup(git_repository *repo, std::string_view rev) {
    commit = fromoid(repo, rev);
    if (commit == nullptr && (commit = frombranch(repo, rev)) == nullptr) {
      fprintf(stderr, "rev: '%s' not commit and not branch\n", rev.data());
      return false;
    }
    if (git_commit_tree(&tree, commit) != 0) {
      auto ec = giterr_last();
      fprintf(stderr, "unable open commit  tree'%s'\n", rev.data());
      if (ec != nullptr) {
        fprintf(stderr, "last error: %s\n", ec->message);
      }
      return false;
    }
    return true;
  }
};

class treewalk_base {
public:
  treewalk_base() {
    //
    git_libgit2_init();
  }
  ~treewalk_base() {
    if (repo_ != nullptr) {
      git_repository_free(repo_);
    }
    git_libgit2_shutdown();
  }
  bool open(std::string_view dir);
  bool walk(std::string_view rev);
  git_repository *repo() const { return repo_; }

private:
  git_repository *repo_{nullptr};
};

bool treewalk_base::open(std::string_view dir) {
  if (git_repository_open(&repo_, dir.data()) != 0) {
    auto ec = giterr_last();
    fprintf(stderr, "unable open repository '%s'\n", dir.data());
    if (ec != nullptr) {
      fprintf(stderr, "last error: %s\n", ec->message);
    }
    return false;
  }
  return true;
}

int git_treewalk_impl(const char *root, const git_tree_entry *entry,
                      void *payload) {
  (void)payload;
  std::string name = root;
  name.append(git_tree_entry_name(entry));
  fprintf(stderr, "%s\n", name.data());
  return 0;
}

bool treewalk_base::walk(std::string_view rev) {
  treebase_t tb;
  if (!tb.lookup(repo_, rev)) {
    return false;
  }
  if (git_tree_walk(tb.tree, GIT_TREEWALK_PRE, git_treewalk_impl, nullptr) !=
      0) {
    return false;
  }
  return true;
}

int main(int argc, char const *argv[]) {
  if (argc < 3) {
    fprintf(stderr, "usage: %s gitdir commit\n", argv[0]);
    return 1;
  }
  treewalk_base base;
  if (!base.open(argv[1])) {
    return 1;
  }
  if (!base.walk(argv[2])) {
    return 1;
  }
  return 0;
}
