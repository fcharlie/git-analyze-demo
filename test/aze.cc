///
#include <string>
#include <cstdio>
#include <cstdlib>
#include <git2.h>

class RiiaRepository {
public:
  RiiaRepository(const char *dir) {
    if (git_repository_open(&repo_, dir) != 0) {
      // throw std::runtime
    }
  }
  ~RiiaRepository() {
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
  return 0;
}
