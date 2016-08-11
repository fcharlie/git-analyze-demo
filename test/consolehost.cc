////
#include <Windows.h>
#include <io.h>
#include <stdio.h>
#include <stdlib.h>
/// check Stdhandle
bool IsUnderConsoleHost() {
  // HANDLE hStderr = fdtoh(STDERR_FILENO);
  //_isatty()
  // return (GetFileType(hStderr) == FILE_TYPE_CHAR);
  return _isatty(_fileno(stderr)) != 0;
}

int main() {
  ////
  if (IsUnderConsoleHost()) {
    MessageBoxW(nullptr, L"Run Under ConsoleHost", L"Title", MB_OK);
  } else {
    MessageBoxW(nullptr, L"Not ConsoleHost", L"Title", MB_OK);
  }
  return 0;
}
