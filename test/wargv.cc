
#include <string>
#include <string_view>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <utf.h>

char *u16tou8(const wchar_t *arg) {
  std::u16string u16 = reinterpret_cast<const char16_t *>(arg);
  auto s = utf::utf16_to_utf8(u16.begin(), u16.end());
  return strdup(s.data());
}

class argv_container {
private:
  void release() {
    for (auto a : argv_) {
      free(a);
    }
    argv_.clear();
  }

public:
  argv_container() = default;
  argv_container(argv_container &&other) {
    release();
    argv_.assign(other.argv_.begin(), other.argv_.end());
    other.argv_.clear();
  }
  argv_container(const argv_container &) = delete;
  argv_container &operator=(const argv_container &) = delete;
  ~argv_container() { release(); }
  int argc() const { return argv_.size(); }
  char **argv() const { return const_cast<char **>(argv_.data()); }
  static argv_container make(int argc, wchar_t **wargv) {
    argv_container avc;
    for (int i = 0; i < argc; i++) {
      avc.argv_.push_back(u16tou8(wargv[i]));
    }
    avc.argv_.push_back(nullptr);
    return avc;
  }

private:
  std::vector<char *> argv_;
};

int cmd_main(int argc, char **argv) {
  for (int i = 0; i < argc; i++) {
    fprintf(stderr, "%s\n", argv[i]);
  }
  return 0;
}

int wmain(int argc, wchar_t **argv) {
  auto avc = argv_container::make(argc, argv);
  return cmd_main(avc.argc(), avc.argv());
}
