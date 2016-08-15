////
#include <cstdio>
#include <Pal.hpp>

int main() {
  PalEnvironment env;
#ifdef _WIN32
  fprintf(stderr, "%d", env.Integer(L"NUMBER_OF_PROCESSORS", 1));
#else
  fprintf(stderr, "%s\n", env.Strings("TERM").c_str());
#endif
  return 0;
}