// internal don't include this
#include <git2.h>

class RaiiRepository {
public:
  RaiiRepository() : repo_(nullptr), cur_commit_(nullptr) {}
  ~RaiiRepository() {
    if (cur_commit_) {
      git_commit_free(cur_commit_);
    }
    if (repo_) {
      git_repository_free(repo_);
    }
  }
  bool refcommit(const char *refname);
  bool walk();
  bool load(const char *dir);
  git_repository *repository() const { return repo_; }
  git_commit *commit() const { return cur_commit_; }

private:
  git_repository *repo_;
  git_commit *cur_commit_;
};
