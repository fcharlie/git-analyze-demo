/*
* complete.cc
* git-analyze
* author: Force.Charlie
* Date: 2016.12
* Copyright (C) 2016. OSChina.NET. All Rights Reserved.
*/
/// to create total year commits
#include <git2.h>
#include <ctime>
#include <cstdio>
#include <cstring>
#include <string>
#include <Pal.hpp>

inline unsigned Days(unsigned year) {
  if ((year % 4 == 0 && year % 100 != 0) || year % 400 == 0)
    return 366;
  return 365;
}

class YearComplete {
public:
  YearComplete() {
    memset(name, 0, sizeof(name));
    memset(email, 0, sizeof(email));
  }
  ~YearComplete() {
    if (parent) {
      git_commit_free(parent);
    }
    if (repo) {
      git_repository_free(repo);
    }
  }
  bool InitializeRepository(const char *dir, const char *ref,
                            const char *msgTemplate) {
    if (git_repository_open(&repo, dir) != 0)
      return false;
    if (!DiscoverUsernameEmail()) {
      BaseErrorMessagePrint("Not Found configured name and email\n");
      return false;
    }
    if (!FindHead())
      return false;
    refs.assign(ref);
    message.assign(msgTemplate);
    return true;
  }
  bool FillYear(unsigned year) {
    auto t = time(nullptr);
    auto p = localtime(&t);
    unsigned my = year <= 1900 ? p->tm_year : year - 1900;
    struct tm mt;
    memset(&mt, 0, sizeof(tm));
    auto days = Days(my + 1900);
    mt.tm_mday = 1;
    mt.tm_mon = 0;
    mt.tm_hour = p->tm_hour;
    mt.tm_min = p->tm_min;
    mt.tm_sec = p->tm_sec;
    mt.tm_year = my;

    git_time gt;
    gt.time = mktime(&mt);
    gt.offset = 0;
    FillFirstCommit(gt, message.c_str());
    for (unsigned i = 2; i <= days; i++) {
      mt.tm_mday = i;
      mt.tm_mon = 0;
      mt.tm_hour = p->tm_hour;
      mt.tm_min = p->tm_min;
      mt.tm_sec = p->tm_sec;
      mt.tm_year = my;
      gt.time = mktime(&mt);
      FillDateCommit(gt, message.c_str());
    }
    return true;
  }

private:
  std::string refs;
  std::string message{"no commit message"};
  char name[256];
  char email[256];
  git_repository *repo{nullptr};
  git_commit *parent{nullptr};
  /// get user name and email
  bool DiscoverUsernameEmail() {
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
  bool FindHead() {
    git_reference *ref = nullptr;
    if (git_repository_head(&ref, repo) != 0) {
      auto err = giterr_last();
      BaseErrorMessagePrint("cannot open head %s\n", err->message);
      git_repository_free(repo);
      return false;
    }
    git_reference *xref{nullptr};
    if (git_reference_resolve(&xref, ref) != 0) {
      auto err = giterr_last();
      BaseErrorMessagePrint("Resolve reference: %s\n", err->message);
      git_reference_free(ref);
      git_repository_free(repo);
      return false;
    }
    auto oid = git_reference_target(xref);
    if (!oid) {
      auto err = giterr_last();
      BaseErrorMessagePrint("Lookup commit: %s\n", err->message);
      git_reference_free(xref);
      git_reference_free(ref);
      return false;
    }
    if (git_commit_lookup(&parent, repo, oid) != 0) {
      auto err = giterr_last();
      BaseErrorMessagePrint("Lookup commit tree: %s\n", err->message);
      git_reference_free(xref);
      git_reference_free(ref);
      return false;
    }
    git_reference_free(xref);
    git_reference_free(ref);
    return true;
  }
  bool FillFirstCommit(git_time when, const char *msg) {
    git_signature sig;
    sig.name = name;
    sig.email = email;
    sig.when = when;
    git_oid newoid;
    git_tree *tree = nullptr;
    if (git_commit_tree(&tree, parent) != 0) {
      auto err = giterr_last();
      BaseErrorMessagePrint("git_commit_tree() %s\n", err->message);
      return false;
    }

    if (git_commit_create(&newoid, repo, refs.c_str(), &sig, &sig, NULL, msg,
                          tree, 0, nullptr) != 0) {
      auto err = giterr_last();
      BaseErrorMessagePrint("git_commit_create() %s\n", err->message);
      git_tree_free(tree);
      return false;
    }
    git_tree_free(tree);
    git_commit_free(parent);
    parent = nullptr;
    if (git_commit_lookup(&parent, repo, &newoid)) {
      return false;
    }
    return true;
  }

  bool FillDateCommit(git_time when, const char *msg) {
    git_signature sig;
    sig.name = name;
    sig.email = email;
    sig.when = when;
    git_oid newoid;
    git_tree *tree = nullptr;
    if (git_commit_tree(&tree, parent) != 0) {
      auto err = giterr_last();
      BaseErrorMessagePrint("git_commit_tree() %s\n", err->message);
      return false;
    }
    const git_commit *ps[] = {parent};
    if (git_commit_create(&newoid, repo, refs.c_str(), &sig, &sig, NULL, msg,
                          tree, 1, ps) != 0) {
      auto err = giterr_last();
      BaseErrorMessagePrint("git_commit_create() %s\n", err->message);
      git_tree_free(tree);
      return false;
    }
    git_tree_free(tree);
    git_commit_free(parent);
    parent = nullptr;
    if (git_commit_lookup(&parent, repo, &newoid)) {
      return false;
    }
    return true;
  }
};

class GitInit {
public:
  GitInit() { git_libgit2_init(); }
  ~GitInit() { git_libgit2_shutdown(); }

private:
};

//// this is real main
int Main(int argc, char **argv) {
  GitInit ginit;
  int year = 0;
  if (argc < 3) {
    BaseErrorMessagePrint(
        "usage: %s  dir branch message year\nExample: git-complete "
        ". v2016 'no commit message' 2016 \n",
        argv[0]);
    return 1;
  }

  YearComplete yearComplete;
  std::string ref("refs/heads/");
  ref.append(argv[2]);
  yearComplete.InitializeRepository(argv[1], ref.c_str(), argv[3]);
  if (argc > 4) {
    char *c = nullptr;
    year = strtol(argv[4], &c, 10);
  }
  yearComplete.FillYear(year);
  return 0;
}

#if defined(_WIN32) && !defined(__CYGWIN__)
//// When use Visual C++, Support convert encoding to UTF8
#include <stdexcept>
#include <Windows.h>
//// To convert Utf8
char *CopyToUtf8(const wchar_t *wstr) {
  auto l = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, NULL, 0, NULL, NULL);
  char *buf = (char *)malloc(sizeof(char) * l + 1);
  if (buf == nullptr)
    throw std::runtime_error("Out of Memory ");
  WideCharToMultiByte(CP_UTF8, 0, wstr, -1, buf, l, NULL, NULL);
  return buf;
}
int wmain(int argc, wchar_t **argv) {
  std::vector<char *> Argv_;
  auto Release = [&]() {
    for (auto &a : Argv_) {
      free(a);
    }
  };
  try {
    for (int i = 0; i < argc; i++) {
      Argv_.push_back(CopyToUtf8(argv[i]));
    }
  } catch (const std::exception &e) {
    BaseErrorMessagePrint("Exception: %s\n", e.what());
    Release();
    return -1;
  }
  auto result = Main(Argv_.data(), Argv_.size());
  Release();
  return result;
}
#else
int main(int argc, char **argv) {
  /* code */
  return Main(argc, argv);
}
#endif
