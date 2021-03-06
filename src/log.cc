/*
 * log.cc
 *
 *  Created on: Mar 26, 2017
 *      Author: zhanghao
 */

#include <cstdarg>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <ctime>
#include <execinfo.h>
#include <stdio.h>
#include <cstdlib>
#include "log.h"
#include "settings.h"

namespace httpserver {
static char* LOGFILE = nullptr;
void _LogRaw(int level, const char* msg) {
  const char *c = ".-*#";
  FILE *fp;
  char buf[128];

  fp = (LOGFILE == nullptr) ?
          (LOG_LEVEL <= LOG_FATAL ? stderr : stdout) : fopen(LOGFILE, "a");
  if (!fp) return;

  int off;
  struct timeval tv;

  gettimeofday(&tv, NULL);
  off = strftime(buf, sizeof(buf), "%d %b %H:%M:%S.", localtime(&tv.tv_sec));
  snprintf(buf + off, sizeof(buf) - off, "%03d", (int) tv.tv_usec / 1000);
  fprintf(fp, "[%d] %s %c %s\n", (int) syscall(SYS_gettid), buf, c[level], msg);

  fflush(fp);

  if (LOGFILE) fclose(fp);
}

void _Log(char* file, char* func, int lineno, int level, const char *fmt, ...) {
  if (level > LOG_LEVEL)
    return;

  va_list ap;
  char msg[MAX_LOGMSG_LEN];

  int n = sprintf(msg, "[%s:%d-%s()] ", file, lineno, func);
  va_start(ap, fmt);
  vsnprintf(msg + n, MAX_LOGMSG_LEN - n, fmt, ap);
  va_end(ap);

  _LogRaw(level, msg);
}
}

