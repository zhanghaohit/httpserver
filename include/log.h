/*
 * debug.h
 *
 *  Created on: Mar 1, 2016
 *      Author: zhanghao
 */

#ifndef INCLUDE_DEBUG_H_
#define INCLUDE_DEBUG_H_

#include <cassert>

namespace httpserver {
#define LOG_FATAL 0
#define LOG_WARNING 1
#define LOG_INFO 2
#define LOG_DEBUG 3

#define MAX_LOGMSG_LEN    1024 /* Default maximum length of syslog messages */

#if defined __cplusplus
# define __ASSERT_VOID_CAST static_cast<void>
#else
# define __ASSERT_VOID_CAST (void)
#endif

void _Log(char* file, char *func, int lineno, int level, const char *fmt, ...);
void StackTrace();

#define LOG(level, fmt, ...) _Log ((char*)__FILE__, (char*)__func__, __LINE__, level, fmt, ## __VA_ARGS__)

#ifdef NDEBUG
#define Assert(_e) (__ASSERT_VOID_CAST (0))
#else
#define Assert(_e) ((_e)?(void)0 : (LOG(LOG_WARNING, #_e" Assert Failed"),StackTrace(),assert(false)))
#endif
#define Panic(fmt, ...) Log(LOG_FATAL, fmt, ##__VA_ARGS__),exit(1)
} //end of namespace

#endif /* INCLUDE_DEBUG_H_ */
