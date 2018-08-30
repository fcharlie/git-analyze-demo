////
#ifndef GIT_ANALYZE_HPP
#define GIT_ANALYZE_HPP
#include <string>
#include <cstring>
#include <vector>
#include <git2.h>

namespace git {

inline bool StartsWith(const std::string &lhs, const char *str,
                       std::size_t len) {
  if (len > lhs.size() || len == 0) {
    return false;
  }
  return (memcmp(lhs.data(), str, len) == 0);
}
inline bool StartsWith(const std::string &lhs, const char *str) {
  if (str == nullptr) {
    return false;
  }
  auto len = strlen(str);
  return StartsWith(lhs, str, len);
}
inline bool StartsWith(const std::string &lhs, const std::string &rhs) {
  return StartsWith(lhs, rhs.data(), rhs.size());
}

class Initializer {
public:
  Initializer(const Initializer &) = delete;
  Initializer &operator=(const Initializer &) = delete;
  Initializer() {
    /// to init
    git_libgit2_init();
  }
  ~Initializer() {
    //
    git_libgit2_shutdown();
  }

private:
};

class Repository {
public:
  Repository() = default;
  Repository(const Repository &) = delete;
  Repository &operator=(const Repository &) = delete;
  ~Repository() {
    if (repo != nullptr) {
      git_repository_free(repo);
    }
  }
  bool open(const std::string &dir) {
    if (git_repository_open(&repo, dir.c_str()) != 0) {
      return false;
    }
    return true;
  }
  bool branch_exists(const std::string &b) {
    git_reference *ref{nullptr};
    if (git_branch_lookup(&ref, repo, b.c_str(), GIT_BRANCH_LOCAL) != 0) {
      return false;
    }
    git_reference_free(ref);
    return true;
  }
  git_repository *pointer() { return repo; }

private:
  ::git_repository *repo{nullptr};
};

class Commit {
public:
  Commit() = default;
  ~Commit() {
    if (commit != nullptr) {
      git_commit_free(commit);
    }
  }
  bool open_oid(git_repository *repo, const git_oid *id) {
    if (git_commit_lookup(&commit, repo, id) != 0) {
      return false;
    }
    return true;
  }
  /// open reference name style ''
  bool open_ref(git_repository *repo, const std::string &refname) {
    git_reference *ref{nullptr};
    if (git_reference_lookup(&ref, repo, refname.c_str()) != 0) {
      return false;
    }
    switch (git_reference_type(ref)) {
    case GIT_REF_OID: {
      auto oid = git_reference_target(ref);
      auto r = open_oid(repo, oid);
      git_reference_free(ref);
      return r;
    }
    case GIT_REF_SYMBOLIC: {
      git_reference *dref{nullptr};
      if (git_reference_resolve(&dref, ref) != 0) {
        git_reference_free(ref);
        return false;
      }
      git_reference_free(ref);
      auto oid = git_reference_target(dref);
      auto r = open_oid(repo, oid);
      git_reference_free(dref);
      return r;
    }
    default:
      break;
    }
    git_reference_free(ref);
    return true;
  }
  bool open(git_repository *repo, const std::string &rbo) {
    git_oid oid;
    if (git_oid_fromstr(&oid, rbo.c_str()) == 0) {
      if (open_oid(repo, &oid)) {
        return true;
      }
    }
    if (StartsWith(rbo, "refs/heads/") || rbo.compare("HEAD") == 0) {
      return open_ref(repo, rbo);
    }
    std::string refname("refs/heads/");
    refname.append(rbo);
    return open_ref(repo, refname);
  }

  git_commit *pointer() { return commit; }

private:
  ::git_commit *commit;
};

class Tree {
public:
  Tree() = default;
  ~Tree() {
    if (tree != nullptr) {
      git_tree_free(tree);
    }
  }
  bool open(git_repository *repo, git_commit *commit, const std::string &path) {
    // std::vector<std::string> pmv;
    git_tree *xtree{nullptr};
    if (git_commit_tree(&xtree, commit) != 0) {
      return false;
    }
    if (path.empty() || path.compare(".") == 0) {
      tree = xtree;
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
    if (git_tree_lookup(&tree, repo, git_tree_entry_id(te)) != 0) {
      git_tree_entry_free(te);
      git_tree_free(xtree);
      return false;
    }
    git_tree_entry_free(te);
    git_tree_free(xtree);
    return true;
  }
  git_tree *pointer() { return tree; }

private:
  git_tree *tree{nullptr};
};

inline void PrintError() {
  fprintf(stderr, "Last Error:%s\n", giterr_last()->message);
}

} // namespace git

#endif
