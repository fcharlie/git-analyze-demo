///
#include <string>
#include <cstdio>
#include <cstdlib>
#include <stdexcept>
#include <git2.h>
#include <git2/diff.h>

class RaiiRepository {
public:
  RaiiRepository(const char *dir) {
    if (git_repository_open(&repo_, dir) != 0) {
      auto err = giterr_last();
      throw std::runtime_error(err->message);
    }
  }
  ~RaiiRepository() {
    if (repo_) {
      git_repository_free(repo_);
    }
  }
  git_repository *Ptr() { return repo_; }

private:
  git_repository *repo_ = nullptr;
};

class RaiiReference {
public:
  RaiiReference(git_repository *repo, const char *refname) {
    //// First use look refs
    if (git_reference_lookup(&ref_, repo, refname) != 0) {
      //// second look branch to ref
      if (git_branch_lookup(&ref_, repo, refname, GIT_BRANCH_LOCAL) != 0) {
        auto err = giterr_last();
        throw std::runtime_error(err->message);
      }
    }
  }
  RaiiReference(const RaiiReference &symbo_) {
    if (git_reference_resolve(&ref_, symbo_.ref_) != 0) {
      auto err = giterr_last();
      throw std::runtime_error(err->message);
    }
  }
  /// Move
  RaiiReference(RaiiReference &&other) {
    ref_ = other.ref_;
    other.ref_ = nullptr;
  }
  ~RaiiReference() {
    if (ref_) {
      git_reference_free(ref_);
    }
  }
  git_reference *Ptr() { return ref_; }

private:
  git_reference *ref_ = nullptr;
};
////////

int git_diff_file_compare(const git_diff_delta *delta, float progress,
                          void *payload) {
  (void)progress;
  if (delta->status == GIT_DELTA_ADDED || delta->status == GIT_DELTA_MODIFIED) {
    /* code */
    git_repository *repo = (git_repository *)payload;
    git_blob *blob = nullptr;
    if (git_blob_lookup(&blob, repo, &(delta->new_file.id)) != 0) {
      return 0;
    }
    auto size = git_blob_rawsize(blob);
    printf("%ld\n", size);
    if (size > 100) {
      fprintf(stderr, "File: %s\n", delta->new_file.path);
    }
  }
  return 0;
}

class RaiiCommit {
public:
  RaiiCommit(git_repository *repo, const git_oid *oid) {
    if (git_commit_lookup(&commit_, repo, oid) != 0) {
      auto err = giterr_last();
      throw std::runtime_error(err->message);
    }
  }
  ~RaiiCommit() {
    if (commit_) {
      git_commit_free(commit_);
    }
  }

  bool Walk(git_repository *repo) {
    ////
    git_commit *parent = nullptr;
    if (git_commit_parent(&parent, commit_, 0) != 0) {
      return false;
    }
    git_tree *otree = nullptr;
    git_tree *ntree = nullptr;
    if (git_commit_tree(&otree, parent) != 0) {
      git_commit_free(parent);
      return false;
    }
    if (git_commit_tree(&ntree, commit_) != 0) {
      git_tree_free(otree);
      git_commit_free(parent);
      return false;
    }
    git_diff_options opts = GIT_DIFF_OPTIONS_INIT;
    // opts.progress_cb = diff_compare;
    // git_diff_init_options(&opts, GIT_DIFF_OPTIONS_VERSION);
    git_diff *diff = nullptr; ////
    if (git_diff_tree_to_tree(&diff, repo, otree, ntree, &opts) != 0) {
      git_tree_free(otree);
      git_tree_free(ntree);
      git_commit_free(parent);
      return false;
    }
    git_diff_foreach(diff, git_diff_file_compare, NULL, NULL, NULL, repo);
    git_diff_free(diff);
    git_tree_free(otree);
    git_tree_free(ntree);
    git_commit_free(commit_);
    commit_ = parent;
    return true;
  }
  git_commit *Ptr() { return commit_; }

private:
  git_commit *commit_ = nullptr;
};

bool Analyze(const char *dir, const char *refname) {
  ///
  try {
    RaiiRepository repo(dir);
    RaiiReference ref(repo.Ptr(), refname);
    RaiiReference rref(ref);
    printf("real: %s\n", git_reference_name(rref.Ptr()));
    ///
    auto oid = git_reference_target(rref.Ptr());
    if (oid == nullptr) {
      fprintf(stderr, "Cannot resolve ref: %s oid\n", refname);
      return false;
    }
    RaiiCommit commit(repo.Ptr(), oid);
    while (commit.Walk(repo.Ptr())) {
      /////
    }
  } catch (const std::exception &e) {
    fprintf(stderr, "Exception: %s\n", e.what());
    return false;
  }
  return true;
}

int main(int argc, char **argv) {
  const char *dir_ = ".";
  const char *ref_ = "HEAD";
  switch (argc) {
  case 0:
    break;
  case 1:
    break;
  case 2:
    dir_ = argv[1];
    break;
  default:
    dir_ = argv[1];
    ref_ = argv[2];
    break;
  }
  git_libgit2_init();
  if (Analyze(dir_, ref_)) {
    printf("Analyze success %s %s\n", dir_, ref_);
  } else {
    fprintf(stderr, "Analyze failed :%s %s\n", dir_, ref_);
  }
  git_libgit2_shutdown();
  return 0;
}
