/*
 * log.h
 *  some log facility functions
 *  Created on: Mar 26, 2017
 *      Author: zhanghao
 */

#ifndef INCLUDE_LOG_H_
#define INCLUDE_LOG_H_

#include <cassert>

namespace httpserver {
#define LOG_FATAL 0
#define LOG_WARNING 1
#define LOG_INFO 2
#define LOG_DEBUG 3

#define MAX_LOGMSG_LEN    1024 // Default maximum length of syslog messages

#if defined __cplusplus
# define __ASSERT_VOID_CAST static_cast<void>
#else
# define __ASSERT_VOID_CAST (void)
#endif

void _Log(char* file, char *func, int lineno, int level, const char *fmt, ...);

#ifdef NDEBUG
#define LOG(level, fmt, ...)
#else
#define LOG(level, fmt, ...) _Log ((char*)__FILE__, (char*)__func__, __LINE__, level, fmt, ## __VA_ARGS__)
#endif

} //end of namespace

#endif // INCLUDE_LOG_H_
