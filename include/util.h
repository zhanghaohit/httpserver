/*
 * util.h
 *
 *  Created on: Mar 28, 2017
 *      Author: zhanghao
 */

#ifndef INCLUDE_UTIL_H_
#define INCLUDE_UTIL_H_

#include <time.h>

namespace httpserver {
#define likely(x)   __builtin_expect(!!(x), 1)
#define unlikely(x)   __builtin_expect(!!(x), 0)

long get_time();

inline bool operator <(const timespec& lhs, const timespec& rhs) {
    if (lhs.tv_sec == rhs.tv_sec) {
        return lhs.tv_nsec < rhs.tv_nsec;
    } else {
        return lhs.tv_sec < rhs.tv_sec;
    }
}
} //end of namespace

#endif /* INCLUDE_UTIL_H_ */
