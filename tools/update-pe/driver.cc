#include <string_view>
#include <unordered_set>
#include <optional>
#include <vector>
#include <cstdio>

inline std::vector<std::string_view> EmailSplit(std::string_view sv) {
  std::vector<std::string_view> output;
  size_t first = 0;
  while (first < sv.size()) {
    const auto second = sv.find_first_of(';', first);
    if (first != second) {
      auto s = sv.substr(first, second - first);
      output.emplace_back(s);
    }
    if (second == std::string_view::npos) {
      break;
    }
    first = second + 1;
  }
  return output;
}

struct private_emails {
  std::unordered_set<std::string> emails;
};

std::optional<private_emails> resolve_emails() {
  auto ce = std::getenv("GITEE_PRIVATE_EMAILS");
  if (ce == nullptr) {
    return std::nullopt;
  }
  auto ev = EmailSplit(ce);
  if (ev.empty()) {
    return std::nullopt;
  }

  private_emails pe;
  for (auto e : ev) {
    pe.emails.insert(std::string(e));
  }
  return std::make_optional(pe);
}

int cmd_main(int argc, char **argv) {
  if (argc < 4) {
    fprintf(stderr, "usage: %s refname oldrev newrev\n", argv[0]);
    return 1;
  }
  auto ppe = resolve_emails();
  if (!ppe) {
    // not set ppe.
    return 0;
  }
  //
  return 0;
}
