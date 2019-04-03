#include <string_view>
#include <optional>
#include <vector>
#include <cstdio>
#include <git.hpp>
#include "executor.hpp"

int cmd_main(int argc, char **argv) {
  if (argc < 4) {
    fprintf(stderr, "usage: %s refname oldrev newrev\n", argv[0]);
    return 1;
  }
  git::global_initializer_t gi;
  aze::Executor e;
  if (!e.Initialize()) {
    // GITEE_PEA not set 'email0@domain.com;email1@domain.com'
    return 0;
  }
  if (e.Execute(".", argv[2], argv[3])) {

    return 0;
  }
  // Found Private email broken.
  if (e.IsBlocked()) {
    fprintf(stderr,
            "error: GE007: Your push would publish a private email "
            "address.\nYour can make your email public or disable this "
            "protection by visiting:\nhttps://gitee.com/profile/emails\n");
  }
  return 1;
}
