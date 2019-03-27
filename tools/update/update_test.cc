////
#include "engine.hpp"

int main() {
  RulesEngine re;
  re.AddPrefix("lib/");
  re.AddPrefix("usr/bin/");
  constexpr const char *dirs[] = {"lib", "lib64", "lib/libc++.a",
                                  "usr/bin/bash", "var/log"};
  for (auto d : dirs) {
    if (re.FullMatch(d)) {
      fprintf(stderr, "Path '%s' is readonly\n", d);
    } else {
      fprintf(stderr, "Path '%s' is writeable\n", d);
    }
  }
  /* code */
  return 0;
}
