/*
* Pal.hpp
* git-analyze Pal
* author: Force.Charlie
* Date: 2016.08
* Copyright (C) 2016. OSChina.NET. All Rights Reserved.
*/
#ifndef GIT_ANALYZE_PAL_HPP
#define GIT_ANALYZE_PAL_HPP
#include <stdint.h>
#include <stddef.h>

#ifdef __GNUC__
// int BaseBufferPrint(char *buf, size_t maxlen, const char *format, ...)
//     __attribute__((format(printf, 3, 4)));
int BaseErrorMessagePrint(const char *format, ...)
    __attribute__((format(printf, 1, 2)));
int BaseConsoleWrite(const char *format, ...)
    __attribute__((format(printf, 1, 2)));
#elif defined(_MSC_VER) && _MSC_VER > 1600
#include <Sal.h>
int BaseErrorMessagePrint(_Printf_format_string_ const char *format, ...);
int BaseConsoleWrite(_Printf_format_string_ const char *format, ...);
#else
int BaseErrorMessagePrint(const char *format, ...);
int BaseConsoleWrite(const char *format, ...);
#endif

#endif
