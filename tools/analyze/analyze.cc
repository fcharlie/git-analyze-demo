/*
* analyze.cc
* git-analyze
* author: Force.Charlie
* Date: 2016.08
* Copyright (C) 2016. OSChina.NET. All Rights Reserved.
*/
#include <git2.h>
#include "analyze.hpp"

#ifdef _WIN32
#include <Windows.h>

VOID WINAPI OnTimerAPCProc(_In_opt_ LPVOID lpArgToCompletionRoutine,
                           _In_ DWORD dwTimerLowValue,
                           _In_ DWORD dwTimerHighValue) {
  ////
  fprintf(stderr, "git-analyze process timeout, exit !\n");
  exit(-1);
}

bool InitializeTaskTimer(std::int64_t t_) {
  //
  auto hTimer = CreateWaitableTimer(NULL, FALSE, NULL);
  LARGE_INTEGER dueTime;
  dueTime.QuadPart = t_ * -10000000;
  if (hTimer == nullptr)
    return false;
  if (!SetWaitableTimer(hTimer, &dueTime, 0, OnTimerAPCProc, NULL, FALSE)) {
    return false;
  }
  return true;
}

#else
#include <unistd.h>
#include <sys/signal.h>

void TimerSignalEvent(int sig) {
  fprintf(stderr, "git-analyze process timeout, exit !\n");
  exit(-1);
}

bool InitializeTaskTimer(std::int64_t t_) {
  signal(SIGALRM, TimerSignalEvent);
  alarm(static_cast<unsigned int>(t_));
  return true;
}

#endif

// CreateTimerQueueTimer

bool ProcessAnalyzeTask(const AnalyzeArgs &analyzeArgs) {
  if (analyzeArgs.timeout != -1) {
    if (!InitializeTaskTimer(analyzeArgs.timeout)) {
      fprintf(stderr, "create timer failed !\n");
    }
  }
  return true;
}
