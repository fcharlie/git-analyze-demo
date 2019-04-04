/////////////
#include "packbuilder.hpp"

// revwalk one ref
bool PackBuilder::Revwalkone(git_reference *ref) {
  revwalk_t w;
  if (!w.initialize_ref(r, ref)) {
    return false;
  }
  git_oid oid;
  while (git_revwalk_next(&oid, w.walk) == 0) {
    if (git_packbuilder_insert_commit(pb, &oid) != 0) {
      return false;
    }
  }
  return true;
}

int git_packbuilder_progress_impl(int stage, uint32_t current, uint32_t total,
                                  void *payload) {
  //
  return 0;
}

bool PackBuilder::Executor() {
  if (git_packbuilder_new(&pb, r) != 0) {
    return false;
  }
  if (git_packbuilder_set_callbacks(pb, git_packbuilder_progress_impl, this) !=
      0) {
    return false;
  }
  if (git_reference_iterator_new(&it, r) != 0) {
    return false;
  }
  git_reference *cur = nullptr;
  while (true) {
    auto ec = git_reference_next(&cur, it);
    if (ec == GIT_ITEROVER) {
      break;
    }
    if (ec != 0) {
      return false;
    }
    if (!Revwalkone(cur)) {
      return false;
    }
  }
  ///////////////
  ///
  return true;
}
