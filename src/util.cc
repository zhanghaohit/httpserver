/*
 * util.cc
 *
 *  Created on: Mar 28, 2017
 *      Author: zhanghao
 */

#include "util.h"

namespace httpserver {
struct timespec init_time;
void init() __attribute__ ((constructor));
void fini() __attribute__ ((destructor));
void init() {
  clock_gettime(CLOCK_REALTIME, &init_time);
}

void fini() {}

/*
 * initial time that is used to avoid long overflow
 * return the current time in nanoseconds
 */
long get_time() {
  struct timespec start;
  clock_gettime(CLOCK_REALTIME, &start);
  return (start.tv_sec - init_time.tv_sec) * 1000l * 1000 * 1000
      + (start.tv_nsec - init_time.tv_nsec);;
}
} //end of namespace

