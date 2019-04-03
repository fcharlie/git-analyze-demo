////
#if defined(_WIN32) && !defined(__CYGWIN__)

#include <string>
#include <string_view>
#include <cstdlib>
#include <cstring>
#include <vector>
#ifndef _WINDOWS_
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN //
#endif
#include <Windows.h>
#endif

inline char *u16tou8(std::wstring_view wstr) {
  auto l = WideCharToMultiByte(CP_UTF8, 0, wstr.data(), (int)wstr.size(),
                               nullptr, 0, nullptr, nullptr);
  auto buffer = reinterpret_cast<char *>(malloc(l + 2));
  auto N = WideCharToMultiByte(CP_UTF8, 0, wstr.data(), (int)wstr.size(),
                               buffer, l + 1, nullptr, nullptr);
  buffer[N] = 0;
  return buffer;
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

  argv_container(const argv_container &) = delete;
  argv_container &operator=(const argv_container &) = delete;
  argv_container(argv_container &&other) {
    release();
    argv_.assign(other.argv_.begin(), other.argv_.end());
    other.argv_.clear();
  }
  ~argv_container() {
    //
    release();
  }
  int argc() const { return (int)argv_.size(); }
  char **argv() const { return const_cast<char **>(argv_.data()); }
  static argv_container make(int argc, wchar_t **wargv) {
    argv_container avc;
    for (int i = 0; i < argc; i++) {
      avc.argv_.push_back(u16tou8(wargv[i]));
    }
    return avc;
  }

private:
  std::vector<char *> argv_;
};

#endif

int cmd_main(int argc, char **argv);

#if defined(_WIN32) && !defined(__CYGWIN__)
int wmain(int argc, wchar_t **argv) {
  auto avc = argv_container::make(argc, argv);
  return cmd_main(avc.argc(), avc.argv());
}
#else

int main(int argc, char **argv) {
  //
  return cmd_main(argc, argv);
}
#endif
