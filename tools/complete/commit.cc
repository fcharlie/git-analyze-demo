/*
 * commit.cc
 * git-analyze
 * author: Force.Charlie
 * Date: 2017.04
 * Copyright (C) 2019. GITEE.COM. All Rights Reserved.
 */
#include "complete.hpp"

bool Demolisher::Userinfo() {
  git_config *config = nullptr;
  /// To get default....
  if (git_config_open_default(&config) == 0) {
    git_config_entry *entry;
    if (git_config_get_entry(&entry, config, "user.name") == 0) {
      strncpy(name, entry->value, 256);
      git_config_entry_free(entry);
    }
    if (git_config_get_entry(&entry, config, "user.email") == 0) {
      strncpy(email, entry->value, 256);
      git_config_entry_free(entry);
    }
    git_config_free(config);
  }
  std::string path(git_repository_path(repo));
  path.append("/config");
  if (git_config_open_ondisk(&config, path.c_str()) == 0) {
    git_config_entry *entry;
    if (git_config_get_entry(&entry, config, "user.name") == 0) {
      strncpy(name, entry->value, 256);
      git_config_entry_free(entry);
    }
    if (git_config_get_entry(&entry, config, "user.email") == 0) {
      strncpy(email, entry->value, 256);
      git_config_entry_free(entry);
    }
    git_config_free(config);
  }
  if (strlen(name) == 0 || strlen(email) == 0)
    return false;
  return true;
}

bool Demolisher::InitializeRoot() {
  git_reference *ref = nullptr;
  if (git_repository_head(&ref, repo) != 0) {
    auto err = giterr_last();
    Printe("cannot open current head: %s\n", err->message);
    git_repository_free(repo);
    return false;
  }
  git_reference *xref{nullptr};
  if (git_reference_resolve(&xref, ref) != 0) {
    auto err = giterr_last();
    Printe("Resolve reference: %s\n", err->message);
    git_reference_free(ref);
    git_repository_free(repo);
    return false;
  }
  auto oid = git_reference_target(xref);
  if (!oid) {
    auto err = giterr_last();
    Printe("Lookup commit: %s\n", err->message);
    git_reference_free(xref);
    git_reference_free(ref);
    return false;
  }
  if (git_commit_lookup(&parent, repo, oid) != 0) {
    auto err = giterr_last();
    Printe("Lookup commit tree: %s\n", err->message);
    git_reference_free(xref);
    git_reference_free(ref);
    return false;
  }
  git_reference_free(xref);
  git_reference_free(ref);
  return true;
}

/// because call by ourself, not check nullptr
bool Demolisher::Initialize(const char *dir, const char *ref,
                            const char *messageTemplate) {
  if (strncmp(ref, "refs/heads/", sizeof("refs/heads") - 1) == 0) {
    refs.assign(ref);
  } else {
    refs.assign("refs/heads/").append(ref);
  }
  if (messageTemplate) {
    message.assign(messageTemplate);
  }
  if (git_repository_open_ext(&repo, dir, 0, nullptr) != 0) {
    auto err = giterr_last();
    Printe("Open repository %s\n", err->message);
    return false;
  }
  if (!Userinfo()) {
    Printe("Not Found configured name and email\n");
    return false;
  }
  if (!InitializeRoot()) {
    Printe("Current no commit\n");
    return false;
  }
  return true;
}

bool Demolisher::CommitBuilder(git_time when) {
  git_signature sig;
  sig.name = name;
  sig.email = email;
  sig.when = when;
  git_oid newoid;
  git_tree *tree = nullptr;
  if (git_commit_tree(&tree, parent) != 0) {
    auto err = giterr_last();
    Printe("git_commit_tree() %s\n", err->message);
    return false;
  }
  if (!createbranch) {
    const git_commit *ps[] = {parent};
    if (git_commit_create(&newoid, repo, refs.c_str(), &sig, &sig, NULL,
                          message.data(), tree, 1, ps) != 0) {
      auto err = giterr_last();
      Printe("git_commit_create() %s\n", err->message);
      git_tree_free(tree);
      return false;
    }
  } else {
    if (git_commit_create(&newoid, repo, refs.c_str(), &sig, &sig, NULL,
                          message.data(), tree, 0, nullptr) != 0) {
      auto err = giterr_last();
      Printe("git_commit_create() %s\n", err->message);
      git_tree_free(tree);
      return false;
    }
  }
  createbranch = false;
  git_tree_free(tree);
  if (parent)
    git_commit_free(parent);
  parent = nullptr;
  if (git_commit_lookup(&parent, repo, &newoid)) {
    return false;
  }
  return true;
}

static unsigned int Random() {
  static unsigned int g_seed = 0;
  g_seed = (214013 * g_seed + 2531011);
  return (g_seed >> 16) & 0x7FFF;
}

bool Demolisher::RoundYear(int year) {
  auto t = time(nullptr);
  auto p = localtime(&t);
  unsigned my = year <= 1900 ? p->tm_year : year - 1900;
  struct tm mt;
  memset(&mt, 0, sizeof(tm));
  auto days = Days(year);
  git_time gt;
  for (unsigned i = 1; i <= days; i++) {
    mt.tm_mday = i;
    mt.tm_mon = 0;
    auto N = Random() % 5 + 1;
    for (unsigned k = 0; k < N; k++) {
      mt.tm_hour = p->tm_hour;
      mt.tm_min = p->tm_min + k;
      mt.tm_sec = p->tm_sec;
      mt.tm_year = my;
      gt.time = mktime(&mt);
      if (!CommitBuilder(gt)) {
        return false;
      }
    }
  }
  return true;
}

bool Demolisher::IntervalFill(unsigned sy, unsigned ey, bool newref) {
  Print("Year: %d~%d\n", sy, ey);

  if (newref)
    createbranch = true;
  for (auto i = sy; i <= ey; i++) {
    if (!RoundYear(i)) {
      return false;
    }
    printf("Fill %d done\n", i);
  }
  return true;
}
