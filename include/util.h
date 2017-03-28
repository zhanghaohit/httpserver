/*
 * util.h
 *
 *  Created on: Mar 28, 2016
 *      Author: zhanghao
 */

#ifndef INCLUDE_UTIL_H_
#define INCLUDE_UTIL_H_

#include <vector>
#include <sstream>
#include <syscall.h>

namespace httpserver {
long get_time();
uint64_t rdtsc();

#define atomic_add(v, i) __sync_fetch_and_add((v), (i))
#define atomic_read(v) __sync_fetch_and_add((v), (0))

inline int GetRandom(int min, int max) {
  static __thread unsigned int tid = (unsigned int) syscall(SYS_gettid);
  int ret = (rand_r(&tid) % (max - min)) + min;
  return ret;
}

inline int GetRandom(int min, int max, unsigned int* seedp) {
  int ret = (rand_r(seedp) % (max - min)) + min;
  return ret;
}
} //end of namespace

#endif /* INCLUDE_UTIL_H_ */
