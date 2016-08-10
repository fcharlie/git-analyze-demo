/*
* gc.cc
* git-rollback
* author: Force.Charlie
* Date: 2016.08
* Copyright (C) 2016. OSChina.NET. All Rights Reserved.
*/
#include <string>
#include <cstring>

/*
* bool GitGCInvoke(const std::string &dir,bool forced);
*
*/
#ifdef _WIN32
#include <Windows.h>

class WCharacters {
private:
  wchar_t *wstr;

public:
  WCharacters(const char *str) : wstr(nullptr) {
    if (str == nullptr)
      return;
    int unicodeLen = ::MultiByteToWideChar(CP_UTF8, 0, str, -1, NULL, 0);
    if (unicodeLen == 0)
      return;
    wstr = new wchar_t[unicodeLen + 1];
    if (wstr == nullptr)
      return;
    wstr[unicodeLen] = 0;
    ::MultiByteToWideChar(CP_UTF8, 0, str, -1, (LPWSTR)wstr, unicodeLen);
  }
  const wchar_t *Get() {
    if (!wstr)
      return nullptr;
    return const_cast<const wchar_t *>(wstr);
  }
  ~WCharacters() {
    if (wstr)
      delete[] wstr;
  }
};

inline bool PathFileIsExistsU(const std::wstring &path) {
  auto i = GetFileAttributesW(path.c_str());
  return INVALID_FILE_ATTRIBUTES != i;
}

inline bool PathRemoveFileSpecU(wchar_t *begin, wchar_t *end) {
  for (; end > begin; end--) {
    if (*end == '/' || *end == '\\') {
      *end = 0;
      return true;
    }
  }
  return false;
}

typedef BOOL(WINAPI *LPFN_ISWOW64PROCESS)(HANDLE, PBOOL);
BOOL IsRunOnWin64() {
  BOOL bIsWow64 = FALSE;
  LPFN_ISWOW64PROCESS fnIsWow64Process = (LPFN_ISWOW64PROCESS)GetProcAddress(
      GetModuleHandleW(L"kernel32"), "IsWow64Process");
  if (NULL != fnIsWow64Process) {
    if (!fnIsWow64Process(GetCurrentProcess(), &bIsWow64)) {
      // handle error
    }
  }
  return bIsWow64;
}
BOOL WINAPI FindGitInstallationLocation(std::wstring &location) {
  // HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\Git_is1
  // InstallLocation
  HKEY hInst = nullptr;
  LSTATUS result = ERROR_SUCCESS;
  const wchar_t *git4win =
      LR"(SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\Git_is1)";
  const wchar_t *installKey = L"InstallLocation";
  WCHAR buffer[4096] = {0};
#if defined(_M_X64)
  if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, git4win, 0, KEY_READ, &hInst) !=
      ERROR_SUCCESS) {
    if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, git4win, 0,
                      KEY_READ | KEY_WOW64_32KEY, &hInst) != ERROR_SUCCESS) {
      // Cannot found msysgit or Git for Windows install
      return FALSE;
    }
  }
#else
  if (IsRunOnWin64()) {
    if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, git4win, 0,
                      KEY_READ | KEY_WOW64_64KEY, &hInst) != ERROR_SUCCESS) {
      if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, git4win, 0, KEY_READ, &hInst) !=
          ERROR_SUCCESS) {
        // Cannot found msysgit or Git for Windows install
        return FALSE;
      }
    }
  } else {
    if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, git4win, 0, KEY_READ, &hInst) !=
        ERROR_SUCCESS) {
      return FALSE;
    }
  }
#endif
  DWORD type = 0;
  DWORD dwSize = 4096 * sizeof(wchar_t);
  result = RegGetValueW(hInst, nullptr, installKey, RRF_RT_REG_SZ, &type,
                        buffer, &dwSize);
  if (result == ERROR_SUCCESS) {
    location.assign(buffer);
  }
  RegCloseKey(hInst);
  return result == ERROR_SUCCESS;
}
////

// ////
// bool search_git_from_path(std::wstring &gitbin) {
//   ///
//   WCHAR buffer[4096] = {0};
//   DWORD dwLength = 0;
//   ////
//   if ((dwLength =
//            SearchPathW(nullptr, L"git", L".exe", 4096, buffer, nullptr)) > 0)
//            {
//     gitbin.assign(buffer, dwLength);
//     return true;
//   }
//   return false;
// }

bool SearchGitForWindowsInstall(std::wstring &gitbin) {
  //
  if (!FindGitInstallationLocation(gitbin))
    return false;
  gitbin.push_back(L'\\');
  gitbin.append(L"git.exe");
  if (PathFileIsExistsU(gitbin))
    return true;
  return false;
}

//
bool GitExecutePathSearchAuto(const wchar_t *cmd, std::wstring &gitbin) {
  //// Self , Path Env,
  if (PathFileIsExistsU(cmd)) {
    gitbin.assign(cmd);
    return true;
  }
  std::wstring Path;
  Path.reserve(0x8000); /// 32767
  ///
  auto len = GetModuleFileNameW(nullptr, &Path[0], 32767);
  if (len > 0) {
    auto end = &Path[0] + len;
    PathRemoveFileSpecU(&Path[0], end);
    gitbin.assign(&Path[0]);
    gitbin.push_back(L'\\');
    gitbin.append(cmd);
    if (PathFileIsExistsU(gitbin))
      return true;
  }
  ///
  GetEnvironmentVariableW(L"PATH", &Path[0], 32767);
  auto iter = &Path[0];
  for (; *iter; iter++) {
    if (*iter == ';') {
      gitbin.push_back(L'\\');
      gitbin.append(cmd);
      if (PathFileIsExistsU(gitbin)) {
        return true;
      }
      gitbin.clear();
    } else {
      gitbin.push_back(*iter);
    }
  }
  return false;
}

/// First search git from path.
bool GitGCInvoke(const std::string &dir, bool forced) {
  ///
  WCharacters wstr(dir.c_str()); /// convert to UTF16
  std::wstring gitbin;
  if (!GitExecutePathSearchAuto(L"git.exe", gitbin)) {
    if (!SearchGitForWindowsInstall(gitbin)) {
      fprintf(stderr, "Not Found any git install in your path or current "
                      "dir,Not found git-for-windows install");
      return false;
    }
  }
  /////////////////////////////////////////////////////////
  std::wstring cmdline;
  cmdline.reserve(0x8000);
  _snwprintf_s(&cmdline[0], 32767, 32767, LR"("%s" gc )", gitbin.c_str());
  if (forced) {
    wcscat_s(&cmdline[0], 32767, L"--prune=now --force");
  }
  STARTUPINFOW si;
  PROCESS_INFORMATION pi;
  ZeroMemory(&si, sizeof(si));
  ZeroMemory(&pi, sizeof(pi));
  si.cb = sizeof(si);
  if (!CreateProcessW(nullptr, &cmdline[0], nullptr, nullptr, FALSE, 0, nullptr,
                      wstr.Get(), &si, &pi)) {
    return false;
  }
  bool result = false;
  if (WaitForSingleObject(pi.hProcess, INFINITE) == WAIT_OBJECT_0) {
    DWORD dwExit = 0;
    if (GetExitCodeProcess(pi.hProcess, &dwExit) && dwExit == 0) {
      result = true;
    }
  }
  CloseHandle(pi.hThread);
  CloseHandle(pi.hProcess);
  return result;
}

#else
#include <sys/stat.h>

bool GitExecutePathSearchAuto(const char *cmd, std::string &gitbin) {
  auto path_ = getenv("PATH");
  for (; *path_; path_++) {
    if (*path_ == ':') {
      gitbin.append(cmd);
      struct stat st;
      if (stat(gitbin.c_str(), &st) == 0 && (st.st_mode & S_IFMT) == S_IFREG) {
        return true;
      }
      gitbin.clear();
    } else {
      gitbin.push_back(*path_);
    }
  }
  return false;
}

bool GitGCInvoke(const std::string &dir, bool forced) {
  std::string gitbin;
  if (!GitExecutePathSearchAuto("git", gitbin)) {
    fprintf(stderr, "Not Found git in your path !\n");
    return false;
  }
  return true;
}

#endif
