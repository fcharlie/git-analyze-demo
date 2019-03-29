///
#ifndef AZE_GIT_IMPL_HPP
#define AZE_GIT_IMPL_HPP
#include <string>
#include <vector>
#include <optional>
#include "../string.hpp"
#include <git2.h>

namespace git {
struct error_code {
  std::string message;
  int ec;
  explicit operator bool() const noexcept { return ec == 0; }
};

// libgit2 initialize helper
class global_initializer_t {
public:
  global_initializer_t() { git_libgit2_init(); }
  ~global_initializer_t() { git_libgit2_shutdown(); }
  global_initializer_t(const global_initializer_t &) = delete;
  global_initializer_t &operator=(const global_initializer_t &) = delete;

private:
};

class repository;
class reference;
class commit;
class treeex;

class commit {
public:
  commit() = default;
  commit(const commit &) = delete;
  commit &operator=(const commit &) = delete;
  commit(commit &&other) {
    if (c) {
      git_commit_free(c);
    }
    c = other.c;
    other.c = nullptr;
  }
  ~commit() {
    if (c != nullptr) {
      git_commit_free(c);
    }
  }
  git_commit *p() const { return c; }
  std::vector<commit> parents() {
    std::vector<commit> cv;
    auto n = git_commit_parentcount(c);
    for (unsigned int i = 0; i < n; i++) {
      commit pc;
      if (git_commit_parent(&pc.c, c, i) != 0) {
        return cv;
      }
      cv.push_back(std::move(pc));
    }
    return cv;
  }

private:
  friend class repository;
  friend class reference;
  git_commit *c{nullptr};
};

class reference {
public:
  reference() = default;
  reference(reference &&other) {
    if (ref_ != nullptr) {
      git_reference_free(ref_);
    }
    ref_ = other.ref_;
    other.ref_ = nullptr;
  }
  reference(const reference &) = delete;
  reference &operator=(const reference &) = delete;
  ~reference() {
    if (ref_ != nullptr) {
      git_reference_free(ref_);
    }
  }
  std::optional<reference> symbolic_target() {
    if (git_reference_type(ref_) != GIT_REFERENCE_SYMBOLIC) {
      return std::nullopt;
    }
    reference dr;
    if (git_reference_resolve(&dr.ref_, ref_) != 0) {
      return std::nullopt;
    }
    return std::make_optional(std::move(dr));
  }
  const git_oid *commitid(git_oid &id) {
    switch (git_reference_type(ref_)) {
    case GIT_REFERENCE_DIRECT:
      return git_reference_target(ref_);
    case GIT_REFERENCE_SYMBOLIC: {
      git_reference *dr = nullptr;
      if (git_reference_resolve(&dr, ref_) != 0) {
        return nullptr;
      }
      auto p = git_reference_target(dr);
      git_oid_cpy(&id, p);
      git_reference_free(dr);
      return &id;
    }
    default:
      break;
    }
    return nullptr;
  }
  git_reference *p() const { return ref_; }

private:
  friend class repository;
  git_reference *ref_{nullptr};
};

// repository helper
class repository {
public:
  repository() = default;
  repository(repository &&other);
  repository(const repository &) = delete;
  repository &operator=(const repository &) = delete;
  ~repository() {
    if (repo_ != nullptr) {
      git_repository_free(repo_);
    }
  }

  git_repository *pointer() { return repo_; }
  std::optional<reference> get_reference(std::string_view refname);
  std::optional<reference> get_branch(std::string_view branch);
  std::optional<commit> get_reference_commit(std::string_view ref);
  std::optional<commit> get_reference_commit_auto(std::string_view ref);
  std::optional<commit> get_commit(std::string_view oid);

  static std::optional<repository> make_repository(std::string_view sv,
                                                   error_code &ec);

private:
  ::git_repository *repo_{nullptr};
};

class tree {
public:
  tree() = default;
  ~tree() {
    if (tree_ != nullptr) {
      git_tree_free(tree_);
    }
  }
  bool open(git_repository *repo, git_commit *commit, const std::string &path) {
    // std::vector<std::string> pmv;
    git_tree *xtree{nullptr};
    if (git_commit_tree(&xtree, commit) != 0) {
      return false;
    }
    if (path.empty() || path.compare(".") == 0) {
      tree_ = xtree;
      return true;
    }
    git_tree_entry *te{nullptr};
    if (git_tree_entry_bypath(&te, xtree, path.c_str()) != 0) {
      git_tree_free(xtree);
      return false;
    }
    if (git_tree_entry_type(te) != GIT_OBJ_TREE) {
      git_tree_entry_free(te);
      git_tree_free(xtree);
      giterr_set_str(GITERR_TREE, "cannot cast tree entry to tree");
      return false;
    }
    if (git_tree_lookup(&tree_, repo, git_tree_entry_id(te)) != 0) {
      git_tree_entry_free(te);
      git_tree_free(xtree);
      return false;
    }
    git_tree_entry_free(te);
    git_tree_free(xtree);
    return true;
  }
  git_tree *pointer() { return tree_; }

private:
  git_tree *tree_{nullptr};
};

} // namespace git

#endif
