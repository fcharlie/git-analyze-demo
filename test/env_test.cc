///
#include <cstdio>
#include <cstdlib>
#include <Pal.hpp>

int main(int argc, char **argv) {
  PalEnvironment env;
  auto pwd = env.Strings("PWD");
  fprintf(stderr, "PWD: %s\n", pwd.c_str());
  setenv("ENVTEST", "1", 1);
  printf("%s\n", getenv("ENVTEST"));
  if (env.Boolean("ENVTEST")) {
    fprintf(stderr, "ENVTEST TRUE\n");
  } else {
    fprintf(stderr, "ENVTEST FALSE\n");
  }
  setenv("ENVTEST", "true", 1);
  printf("%s\n", getenv("ENVTEST"));
  if (env.Boolean("ENVTEST")) {
    fprintf(stderr, "ENVTEST TRUE\n");
  } else {
    fprintf(stderr, "ENVTEST FALSE\n");
  }
  return 0;
}
