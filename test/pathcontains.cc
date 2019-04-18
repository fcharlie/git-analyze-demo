////////
#include <string>
#include <string_view>
#include <cstring>
#include <cstdio>
#include <cstdlib>

// when sub startswith parant
// 1.  size equal --> path equal
// 2.  path[p.size()]=='/' At this point, 'path' is a subpath of p
inline bool IsPathContains(std::string_view parent, std::string_view sub) {
  return (parent.size() <= sub.size() &&
          memcmp(parent.data(), sub.data(), parent.size()) == 0 &&
          (sub.size() == parent.size() || sub[parent.size()] == '/'));
}
struct pv {
  const char *parant;
  const char *sub;
  bool equal;
};
int main() {
  pv pvv[] = {
      {"sources/driver.cc", "sources/driver", false},
      {"sources/driver.cc", "sources/driver.cc", true},
      {"include/absl", "include/absl/strings/string_view.h", true},
      {"include/absl", "include/absl/", true},
  };
  for (auto p : pvv) {
    if (IsPathContains(p.parant, p.sub) == p.equal) {
      fprintf(stderr, "check ok: %s %s contains: %s\n", p.parant, p.sub,
              p.equal ? "true" : "false");
    } else {
      fprintf(stderr, "check failed: %s %s contains: %s\n", p.parant, p.sub,
              p.equal ? "true" : "false");
    }
  }
  return 0;
}
