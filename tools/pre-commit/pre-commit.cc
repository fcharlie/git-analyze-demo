////
/// pre-commit limit file
#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>
#include <git2.h>

#ifdef _MSC_VER
#define strcasecmp stricmp
#endif

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
      if (strcasecmp(c, "K")) {
        l = l * 1024;
      } else if (strcasecmp(c, "M")) {
        l = l * (1UL << 20);
      } else if (strcasecmp(c, "G")) {
        l = l * (1ULL << 30);
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
  std::uint64_t limitSize;
  std::uint64_t warnSize;
  std::vector<std::string> filters;
  bool filterBroken{false};
};

class GitInit {
public:
  GitInit() { git_libgit2_init(); }
  ~GitInit() { git_libgit2_shutdown(); }

private:
};

bool PrecommitExecute(const char *td) {
  git_repository *repo = nullptr;
  if (git_repository_open(&repo, td ? td : ".") != 0) {
    auto err = giterr_last();
    fprintf(stderr, "Open repository: %s\n", err->message);
    return false;
  }
  PrecommitSwitch ps;
  ps.Initialize(git_repository_path(repo));

  git_repository_free(repo);
  return true;
}

int main(int argc, char **argv) {
  GitInit ginit;
  const char *td = nullptr;
  if (argc >= 2) {
    td = argv[1];
  }
  if (!PrecommitExecute(td)) {
    return 1;
  }
  return 0;
}
