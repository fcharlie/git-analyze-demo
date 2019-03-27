////////
#include "executor.hpp"
#include <git2.h>

struct commit_t {
  commit_t() = default;
  ~commit_t() {
    if (tree != nullptr) {
      git_tree_free(tree);
    }
    if (commit != nullptr) {
      git_commit_free(commit);
    }
  }
  git_commit *commit{nullptr};
  git_tree *tree{nullptr};
  bool lookup(git_repository *repo, const git_oid *id) {
    if (git_commit_lookup(&commit, repo, id) != 0) {
      return false;
    }
    return git_commit_tree(&tree, commit) == 0;
  }
};

class executor_base {
public:
  executor_base() {
    //
    git_libgit2_init();
  }
  ~executor_base() {
    if (repo_ != nullptr) {
      git_repository_free(repo_);
    }
    git_libgit2_shutdown();
  }
  bool open(std::string_view path) {
    return git_repository_open(&repo_, path.data()) == 0;
  }
  git_repository *repo() { return repo_; }

private:
  git_repository *repo_{nullptr};
};

Executor::Executor() {
  // initialzie todo
  base = new executor_base();
}
Executor::~Executor() {
  // delete
  delete base;
}

bool Executor::InitializeRules(std::string_view sv, std::string_view ref) {
  if (ref.compare(0, sizeof("refs/heads/") - 1, "refs/heads/") != 0) {
    return true;
  }
  auto branch = ref.substr(sizeof("refs/heads/") - 1);
  return engine.PreInitialize(sv, branch);
}

////////////////////////////////////////////////
int git_treewalk_impl(const char *root, const git_tree_entry *entry,
                      void *payload) {
  auto e = reinterpret_cast<Executor *>(payload);
  std::string name = root;
  name.append(git_tree_entry_name(entry));
  if (e->FullMatch(name)) {
    fprintf(stderr, "Path %s is readonly\n", name.c_str());
    return 1;
  }
  return 0;
}
bool Executor::ExecuteTreeWalk(std::string_view rev) {
  git_oid oid;
  if (git_oid_fromstrn(&oid, rev.data(), rev.size()) != 0) {
    return false;
  }
  commit_t commit;
  if (!commit.lookup(base->repo(), &oid)) {
    return false;
  }
  return (git_tree_walk(commit.tree, GIT_TREEWALK_PRE, git_treewalk_impl,
                        this) == 0);
}

int git_diff_callback(const git_diff_delta *delta, float progress,
                      void *payload) {
  auto e = reinterpret_cast<Executor *>(payload);
  (void)progress;
  if (e->FullMatch(delta->old_file.path)) {
    fprintf(stderr, "Path '%s' is readonly\n", delta->old_file.path);
    return 1;
  }
  if (e->FullMatch(delta->new_file.path)) {
    fprintf(stderr, "Path '%s' is readonly\n", delta->old_file.path);
    return 1;
  }
  return 0;
}

struct diff_t {
  ~diff_t() {
    if (p != nullptr) {
      git_diff_free(p);
    }
  }
  git_diff *p{nullptr};
};

bool Executor::Execute(std::string_view path, std::string_view oldrev,
                       std::string_view newrev) {
  if (engine.Empty()) {
    // Engine rules empty so return
    return true;
  }
  if (!base->open(path)) {
    return false;
  }
  constexpr const char zerooid[] = "0000000000000000000000000000000000000000";
  if (oldrev == zerooid) {
    return ExecuteTreeWalk(newrev);
  }
  if (newrev == zerooid) {
    return ExecuteTreeWalk(oldrev);
  }
  git_oid ooid, noid;
  if (git_oid_fromstrn(&ooid, oldrev.data(), oldrev.size()) != 0) {
    return false;
  }
  if (git_oid_fromstrn(&noid, newrev.data(), newrev.size()) != 0) {
    return false;
  }
  commit_t oldcommit, newcommit;
  if (!oldcommit.lookup(base->repo(), &ooid)) {
    return false;
  }
  if (!newcommit.lookup(base->repo(), &noid)) {
    return false;
  }
  diff_t diff;
  git_diff_options opts;
  git_diff_init_options(&opts, GIT_DIFF_OPTIONS_VERSION);
  if (git_diff_tree_to_tree(&diff.p, base->repo(), oldcommit.tree,
                            newcommit.tree, &opts) != 0) {
    return false;
  }
  return git_diff_foreach(diff.p, git_diff_callback, nullptr, nullptr, nullptr,
                          this) == 0;
}
