////
/// pre-commit limit file
#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>
#include <list>
#include <map>
#include <functional>
#include <git2.h>
#include <Pal.hpp>

#ifdef _MSC_VER
#define strcasecmp stricmp
#endif
#define KBSIZE (1UL << 10)
#define MBSIZE (1UL << 20)
#define GBSIZE (1ULL << 30)
#define TBSIZE (1ULL << 40)
class PrecommitSwitch {
public:
  PrecommitSwitch() {
#ifdef _WIN32
    filters = {"o", "a", "out", "exe", "lib", "pdb"};
#else
    filters = {"o", "a", "out"};
#endif
  }
  std::uint64_t FileSize(std::uint64_t dsize, const char *str) {
    char *c = nullptr;
    auto l = strtoll(str, &c, 10);
    if (c) {
      if (strcasecmp(c, "K") == 0 || strcasecmp(c, "KB") == 0) {
        l = l * KBSIZE;
      } else if (strcasecmp(c, "M") == 0 || strcasecmp(c, "MB") == 0) {
        l = l * MBSIZE;
      } else if (strcasecmp(c, "G") == 0 || strcasecmp(c, "GB") == 0) {
        l = l * GBSIZE;
      }
    }
    if (l != 0)
      return l;
    return dsize;
  }
  bool FileFilter(const std::string &str) {
    std::vector<std::string> v;
    std::size_t start = 0, end = 0;
    while ((end = str.find(';', start)) != std::string::npos) {
      v.push_back(str.substr(start, end - start));
      start = end + 1;
    }
    v.push_back(str.substr(start));
    filters = std::move(v);
    return true;
  }
  bool Initialize(const char *gitdir) {
    ///
    git_config *config = nullptr;
    /// To get default....
    if (git_config_open_default(&config) == 0) {
      git_config_entry *entry;
      if (git_config_get_entry(&entry, config, "commit.limitsize") == 0) {
        limitSize = FileSize(limitSize, entry->value);
        git_config_entry_free(entry);
      }
      if (git_config_get_entry(&entry, config, "commit.warnsize") == 0) {
        warnSize = FileSize(warnSize, entry->value);
        git_config_entry_free(entry);
      }
      if (git_config_get_entry(&entry, config, "commit.filters") == 0) {
        FileFilter(entry->value);
        git_config_entry_free(entry);
      }
      int out;
      if (git_config_get_bool(&out, config, "commit.filterbroken") == 0) {
        filterBroken = out;
      }
      git_config_free(config);
    }
    /// To get local
    std::string path(gitdir);
    path.append("/config");
    if (git_config_open_ondisk(&config, path.c_str()) == 0) {
      git_config_entry *entry;
      if (git_config_get_entry(&entry, config, "commit.limitsize") == 0) {
        limitSize = FileSize(limitSize, entry->value);
        git_config_entry_free(entry);
      }
      if (git_config_get_entry(&entry, config, "commit.warnsize") == 0) {
        warnSize = FileSize(warnSize, entry->value);
        git_config_entry_free(entry);
      }
      if (git_config_get_entry(&entry, config, "commit.filters") == 0) {
        FileFilter(entry->value);
        git_config_entry_free(entry);
      }
      int out;
      if (git_config_get_bool(&out, config, "commit.filterbroken") == 0) {
        filterBroken = out;
      }
      git_config_free(config);
    }
    return true;
  }
  /// git config  commit.limitsize 100M
  std::uint64_t LimitSize() const { return limitSize; }
  /// git config  commit.warnsize 50M
  std::uint64_t WarnSize() const { return warnSize; }
  /// git config  commit.filters '.exe;.lb'
  const std::vector<std::string> Filters() const { return filters; }
  bool FilterBroken() { return filterBroken; }

private:
  std::uint64_t limitSize{100 * MBSIZE};
  std::uint64_t warnSize{50 * MBSIZE};
  std::vector<std::string> filters;
  bool filterBroken{false};
};

class GitInit {
public:
  GitInit() { git_libgit2_init(); }
  ~GitInit() { git_libgit2_shutdown(); }

private:
};

#define DoRelease(x, y)                                                        \
  if (x) {                                                                     \
    y(x);                                                                      \
  }

struct PrecommitInfo {
  PrecommitSwitch ps;
  git_repository *repo;
  std::size_t limitfiles{0};
  std::size_t warnfiles{0};
  std::size_t filterfiles{0};
};

int git_diff_callback(const git_diff_delta *delta, float progress,
                      void *payload) {
  (void)progress;
  if (delta->status == GIT_DELTA_ADDED || delta->status == GIT_DELTA_MODIFIED) {
    /* code */
    // RaiiRepository *repo_ = static_cast<RaiiRepository *>(payload);
    PrecommitInfo *info = static_cast<PrecommitInfo *>(payload);
    git_blob *blob = nullptr;
    if (git_blob_lookup(&blob, info->repo, &(delta->new_file.id)) != 0) {
      return 0;
    }
    // if(strncmp(const char *__s1, const char *__s2, size_t __n))
    //// by default off_t is 8byte
    auto lsize = info->ps.LimitSize();
    auto wsize = info->ps.WarnSize();
    git_off_t size = git_blob_rawsize(blob);
    if (size > info->ps.LimitSize()) {
      ///
      BaseErrorMessagePrint("%s size is %4.2f MB more than %4.2f MB\n",
                            delta->new_file.path, (double)size / MBSIZE,
                            (double)lsize / MBSIZE);
      info->limitfiles++;
    } else if (size > info->ps.WarnSize()) {
      BaseWarningMessagePrint("%s size %4.2f MB more than %4.2f MB\n",
                              delta->new_file.path, (double)size / MBSIZE,
                              (double)wsize / MBSIZE);
      info->warnfiles++;
    }
  }
  return 0;
}

bool PrecommitExecute(const char *td) {
  PrecommitInfo info;
  git_repository *repo = nullptr;
  git_reference *ref = nullptr;
  git_reference *dref = nullptr;
  git_index *index = nullptr;
  git_tree *tree = nullptr;
  git_diff *diff = nullptr;
  git_commit *commit = nullptr;
  const char *errmsg = nullptr;
  bool result = false;
  git_diff_options opts = GIT_DIFF_OPTIONS_INIT;
  if (git_repository_open(&repo, td ? td : ".") != 0) {
    errmsg = "open repository";
    goto Cleanup;
  }
  info.repo = repo;
  info.ps.Initialize(git_repository_path(repo));
  if (git_reference_lookup(&ref, repo, "HEAD") != 0) {
    errmsg = "lookup HEAD";
    goto Cleanup;
  }
  if (git_reference_resolve(&dref, ref) != 0) {
    errmsg = "resolve reference";
    goto Cleanup;
  }
  if (git_commit_lookup(&commit, repo, git_reference_target(dref)) != 0) {
    errmsg = "look commit";
    goto Cleanup;
  }
  if (git_commit_tree(&tree, commit) != 0) {
    errmsg = "commit tree";
    goto Cleanup;
  }
  if (git_repository_index(&index, repo) != 0) {
    errmsg = "repository index";
    goto Cleanup;
  }
  if (git_diff_tree_to_index(&diff, repo, tree, index, &opts) == 0) {
    git_diff_foreach(diff, git_diff_callback, NULL, NULL, NULL, &info);
  }
  if (info.filterfiles != 0 && info.ps.FilterBroken()) {
    BaseErrorMessagePrint("git commit has broken \n");
    BaseWarningMessagePrint("Your can use git rm --cached to remove filter "
                            "files, and commit again !");
    goto Success;
  }
  if (info.limitfiles == 0) {
    result = true;
  } else {
    BaseErrorMessagePrint("git commit has broken \n");
    BaseWarningMessagePrint("Your can use git rm --cached to "
                            "remove large file, and commit again !\n");
  }

  goto Success;
Cleanup:
  if (!result) {
    auto err = giterr_last();
    fprintf(stderr, "LastError %s: %s\n", errmsg, err->message);
  }
Success:
  DoRelease(diff, git_diff_free);
  DoRelease(tree, git_tree_free);
  DoRelease(commit, git_commit_free);
  DoRelease(dref, git_reference_free);
  DoRelease(ref, git_reference_free);
  DoRelease(index, git_index_free);
  DoRelease(repo, git_repository_free);
  return result;
}

int main(int argc, char **argv) {
  GitInit ginit;
  const char *td = nullptr;
  if (argc >= 2) {
    td = argv[1];
  }
  if (!PrecommitExecute(td)) {
    return -1;
  }
  return 0;
}
