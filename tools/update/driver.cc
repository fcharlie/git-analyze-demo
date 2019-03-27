//////////////
#include "executor.hpp"
#include "os.hpp"

/*update hook*/
bool RulesFile(std::string &rf) {
  if (!os::Executable(rf)) {
    fprintf(stderr, "unable lookup exe dir\n");
    return false;
  }
  os::PathRemoveFileSpec(rf);
  rf.push_back(os::PathSeparator);
  rf.append("rw.json");
  return true;
}

// update ref-name oldrev newrev
int main(int argc, char const *argv[]) {
  if (argc < 4) {
    fprintf(stderr, "BAD argv\n");
    return 1;
  }
  std::string rf;
  if (!RulesFile(rf)) {
    return 1;
  }
  Executor e;
  if (!e.InitializeRules(rf, argv[1])) {
    return 1;
  }
  if (!e.Execute(".", argv[2], argv[3])) {
    ///// BAD
    return 1;
  }
  return 0;
}
