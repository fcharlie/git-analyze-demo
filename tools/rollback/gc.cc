/*
* gc.cc
* git-rollback
* author: Force.Charlie
* Date: 2016.08
* Copyright (C) 2016. OSChina.NET. All Rights Reserved.
*/
#include <string>

/*
* bool GitGCInvoke(const std::string &dir,bool prune);
*
*/
#ifdef _WIN32
#include <Windows.h>
typedef BOOL(WINAPI *LPFN_ISWOW64PROCESS)(HANDLE, PBOOL);
BOOL IsRunOnWin64() {
  BOOL bIsWow64 = FALSE;
  LPFN_ISWOW64PROCESS fnIsWow64Process = (LPFN_ISWOW64PROCESS)GetProcAddress(
      GetModuleHandle(TEXT("kernel32")), "IsWow64Process");
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
bool search_git_install(std::wstring &gitbin) {
  //
  return true;
}

//
bool PathSearchAuto() {
  //// Self , Path Env,
  return true;
}

bool search_git_from_path(std::wstring &gitbin) {
  ///
  WCHAR buffer[4096] = {0};
  DWORD dwLength = 0;
  ////
  if ((dwLength =
           SearchPathW(nullptr, L"git", L".exe", 4096, buffer, nullptr)) > 0) {
    gitbin.assign(buffer, dwLength);
    return true;
  }
  return false;
}

/// First search git from path.
bool GitGCInvoke(const std::string &dir, bool prune) {
  ///
  return true;
}

#else
#include <sys/stat.h>

bool PathSearchAuto(const char *cmd) {
  auto path_ = getenv("PATH");
  std::string gitbin;
  for (; *path_; path_++) {
    if (*path_ == ':') {
      gitbin.append(cmd);
      struct stat st;
      if (stat(gitbin.c_str(), &st) == 0) {
        return true;
      }
      gitbin.clear();
    } else {
      gitbin.push_back(*path_);
    }
  }
  return false;
}

bool GitGCInvoke(const std::string &dir, bool prune) {
  if (!PathSearchAuto("git")) {
    fprintf(stderr, "Not Found git in your path !\n");
    return false;
  }
  return true;
}

#endif
