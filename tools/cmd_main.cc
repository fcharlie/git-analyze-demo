////

int cmd_main(int argc, char **argv);
#if defined(_WIN32) && !defined(__CYGWIN__)
//// When use Visual C++, Support convert encoding to UTF8
#include <stdexcept>
#include <windows.h>
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
      if (a != nullptr) {
        free(a);
      }
    }
  };
  try {
    for (int i = 0; i < argc; i++) {
      Argv_.push_back(CopyToUtf8(argv[i]));
    }
    Argv_.push_back(nullptr);
  } catch (const std::exception &e) {
    fprintf(stderr, "%s\n", e.what());
    return -1;
  }
  return cmd_main(Argv_.size() - 1, Argv_.data());
}

#else
int main(int argc, char **argv) {
  //
  return cmd_main(argc, argv);
}
#endif
