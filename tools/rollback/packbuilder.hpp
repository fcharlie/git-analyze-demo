///////////
#ifndef GIT_ROLLBACK_PACKBUILDER_HPP
#define GIT_ROLLBACK_PACKBUILDER_HPP
#include <string>
#include <string_view>
#include <git2.h>

struct revwalk_t {
  revwalk_t() = default;
  revwalk_t(const revwalk_t &) = delete;
  revwalk_t &operator=(const revwalk_t &) = delete;
  revwalk_t(revwalk_t &&other) {
    if (walk != nullptr) {
      git_revwalk_free(walk);
    }
    walk = other.walk;
    other.walk = nullptr;
  }
  ~revwalk_t() {
    if (walk != nullptr) {
      git_revwalk_free(walk);
    }
  }
  bool initialize_ref(git_repository *r, git_reference *ref) {
    if (git_revwalk_new(&walk, r) != 0) {
      return false;
    }
    return (git_revwalk_push_ref(walk, git_reference_name(ref)) == 0);
  }

  git_revwalk *walk{nullptr};
};

// Repack and delete others.
class PackBuilder {
public:
  PackBuilder(git_repository *r_) : r(r_) {}
  ~PackBuilder() {
    if (it != nullptr) {
      git_reference_iterator_free(it);
    }
    if (pb != nullptr) {
      git_packbuilder_free(pb);
    }
    if (r != nullptr) {
      git_repository_free(r);
    }
  }
  PackBuilder(const PackBuilder &) = delete;
  PackBuilder &operator=(const PackBuilder &) = delete;
  bool Executor();

private:
  bool Revwalkone(git_reference *ref);
  git_repository *r{nullptr};
  git_reference_iterator *it{nullptr};
  git_packbuilder *pb{nullptr};
};

#endif
