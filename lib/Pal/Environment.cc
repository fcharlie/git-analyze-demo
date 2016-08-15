/*
* Environment.cc
* git-analyze Pal
* author: Force.Charlie
* Date: 2016.08
* Copyright (C) 2016. OSChina.NET. All Rights Reserved.
*/
#include <Pal.hpp>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#if defined(_WIN32) && !defined(__CYGWIN__)
#include <Windows.h>

template <> bool PalEnvironmentT<wchar_t>::Boolean(const wchar_t *key) {
  wchar_t buf[8];
  if (GetEnvironmentVariableW(key, buf, 8) == 0) {
    return false;
  }
  if (_wcsicmp(buf, L"true") == 0 || _wcsicmp(buf, L"1") == 0)
    return true;
  return false;
}

template <> std::wstring PalEnvironmentT<wchar_t>::Strings(const wchar_t *key) {
  auto len = GetEnvironmentVariableW(key, nullptr, 0);
  if (len == 0)
    return std::wstring();
  wchar_t *buf = new wchar_t[len + 1];
  if (buf == nullptr)
    return std::wstring();
  GetEnvironmentVariableW(key, buf, len + 1);
  std::wstring value(buf, len);
  delete[] buf;
  return value;
}

template <>
int32_t PalEnvironmentT<wchar_t>::Integer(const wchar_t *key, int32_t defva_) {
  wchar_t buf[20];
  if (GetEnvironmentVariableW(key, buf, 20) != 0) {
    wchar_t *c;
    return wcstol(buf, &c, 10);
  }
  return defva_;
}

template <>
uint32_t PalEnvironmentT<wchar_t>::Integer(const wchar_t *key,
                                           uint32_t defva_) {
  wchar_t buf[20];
  if (GetEnvironmentVariableW(key, buf, 20) != 0) {
    wchar_t *c;
    return wcstoul(buf, &c, 10);
  }
  return defva_;
}

template <>
int64_t PalEnvironmentT<wchar_t>::Integer(const wchar_t *key, int64_t defva_) {
  wchar_t buf[64];
  if (GetEnvironmentVariableW(key, buf, 64) != 0) {
    wchar_t *c;
    return wcstoll(buf, &c, 10);
  }
  return defva_;
}

template <>
uint64_t PalEnvironmentT<wchar_t>::Integer(const wchar_t *key,
                                           uint64_t defva_) {
  wchar_t buf[64];
  if (GetEnvironmentVariableW(key, buf, 64) != 0) {
    wchar_t *c;
    return wcstoull(buf, &c, 10);
  }
  return defva_;
}
////
#else

template <> bool PalEnvironmentT<char>::Boolean(const char *key) {
  //
  auto env_ = std::getenv(key);
  //// Flags=true
  //// Flags=1
  if (env_) {
    if (strcmp(env_, "1") == 0 || strcasecmp(env_, "true") == 0)
      return true;
  }
  return false;
}

template <> std::string PalEnvironmentT<char>::Strings(const char *key) {
  auto cstr = std::getenv(key);
  if (cstr)
    return std::string(cstr);
  return std::string();
}

template <>
int32_t PalEnvironmentT<char>::Integer(const char *key, int32_t defva_) {
  char *c;
  auto env_ = std::getenv(key);
  if (env_) {
    auto i = std::strtol(env_, &c, 10);
    return i;
  }
  return defva_;
}

template <>
uint32_t PalEnvironmentT<char>::Integer(const char *key, uint32_t defva_) {
  char *c;
  auto env_ = std::getenv(key);
  if (env_) {
    auto i = std::strtoul(env_, &c, 10);
    return i;
  }
  return defva_;
}

template <>
int64_t PalEnvironmentT<char>::Integer(const char *key, int64_t defva_) {
  char *c;
  auto env_ = std::getenv(key);
  if (env_) {
    auto i = std::strtoll(env_, &c, 10);
    return i;
  }
  return defva_;
  return defva_;
}

template <>
uint64_t PalEnvironmentT<char>::Integer(const char *key, uint64_t defva_) {
  char *c;
  auto env_ = std::getenv(key);
  if (env_) {
    auto i = std::strtoull(env_, &c, 10);
    return i;
  }
  return defva_;
  return defva_;
}

#endif
