/*
 * util.cc
 *
 *  Created on: Mar 28, 2016
 *      Author: zhanghao
 */

#include <sstream>
#include <cstring>
#include <cstdlib>
#include <stdio.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include "util.h"

namespace httpserver {
struct timespec init_time;
void init() __attribute__ ((constructor));
void fini() __attribute__ ((destructor));
void init() {
  clock_gettime(CLOCK_REALTIME, &init_time);
}

void fini() {
}

/*
 * initial time that is used to avoid long overflow
 * return the current time in nanoseconds
 */
long get_time() {
//	struct timeval start;
//	gettimeofday(&start, NULL);
//	return start.tv_sec*1000l*1000+start.tv_usec;
  struct timespec start;
  clock_gettime(CLOCK_REALTIME, &start);
  return (start.tv_sec - init_time.tv_sec) * 1000l * 1000 * 1000
      + (start.tv_nsec - init_time.tv_nsec);;
}


uint64_t rdtsc() {
  unsigned int lo, hi;
  __asm__ __volatile__ ("rdtsc" : "=a" (lo), "=d" (hi));
  return ((uint64_t) hi << 32) | lo;
}
} //end of namespace

