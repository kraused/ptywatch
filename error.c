
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include "error.h"

static char __thread _buf[4096];

static void Report(const char* prefix, const char* file, const char* func, SInt32 line,
                   const char* fmt, va_list vl)
{
        vsnprintf(_buf, sizeof(_buf), fmt, vl);

	fprintf(stderr, "<%s(), %s:%d> %s%s\n", func, file, line, prefix, _buf);
	fflush (stderr);
}

void PtyWatch_Fatal(const char *file, const char *func, SInt32 line, const char *fmt, ...)
{
	va_list vl;

	va_start(vl, fmt);
	Report("fatal: ", file, func, line, fmt, vl);
	va_end(vl);

	abort();
}

void PtyWatch_Error(const char *file, const char *func, SInt32 line, const char *fmt, ...)
{
	va_list vl;

	va_start(vl, fmt);
	Report("error: ", file, func, line, fmt, vl);
	va_end(vl);
}

void PtyWatch_Warn(const char *file, const char *func, SInt32 line, const char *fmt, ...)
{
	va_list vl;

	va_start(vl, fmt);
	Report("warning: ", file, func, line, fmt, vl);
	va_end(vl);
}

void PtyWatch_Log(const char *file, const char *func, SInt32 line, const char *fmt, ...)
{
	va_list vl;

	va_start(vl, fmt);
	Report("", file, func, line, fmt, vl);
	va_end(vl);
}

void PtyWatch_Debug(const char *file, const char *func, SInt32 line, const char *fmt, ...)
{
	va_list vl;

	va_start(vl, fmt);
	Report("debug: ", file, func, line, fmt, vl);
	va_end(vl);
}

