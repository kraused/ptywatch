
#ifndef PTYWATCH_ERROR_H_INCLUDED
#define PTYWATCH_ERROR_H_INCLUDED 1

#include "common.h"

void PtyWatch_Fatal(const char *file, const char *func, SInt32 line, const char *fmt, ...);
void PtyWatch_Error(const char *file, const char *func, SInt32 line, const char *fmt, ...);
void PtyWatch_Warn (const char *file, const char *func, SInt32 line, const char *fmt, ...);
void PtyWatch_Log  (const char *file, const char *func, SInt32 line, const char *fmt, ...);
void PtyWatch_Debug(const char *file, const char *func, SInt32 line, const char *fmt, ...);

#define PTYWATCH_FATAL(FMT, ...)	PtyWatch_Fatal(__FILE__, __func__, __LINE__, FMT, ## __VA_ARGS__)
#define PTYWATCH_ERROR(FMT, ...)	PtyWatch_Error(__FILE__, __func__, __LINE__, FMT, ## __VA_ARGS__)
#define PTYWATCH_WARN(FMT, ...)		PtyWatch_Warn(__FILE__, __func__, __LINE__, FMT, ## __VA_ARGS__)
#define PTYWATCH_LOG(FMT, ...)		PtyWatch_Log(__FILE__, __func__, __LINE__, FMT, ## __VA_ARGS__)
#define PTYWATCH_DEBUG(FMT, ...)	PtyWatch_Debug(__FILE__, __func__, __LINE__, FMT, ## __VA_ARGS__)

#endif

