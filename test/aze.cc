///
#include <string>
#include <cstdio>
#include <cstdlib>
#include <stdexcept>
#include <git2.h>

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

bool Analyze(const char *dir, const char *ref) {
  ///
  try {
    RaiiRepository repo(dir);
    ////
  } catch (const std::exception &e) {
    fprintf(stderr, "RaiiRepository throw: %s\n", e.what());
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
