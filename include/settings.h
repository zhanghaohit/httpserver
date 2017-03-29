/*
 * settings.h
 *
 *  Created on: Mar 29, 2017
 *      Author: zhanghao
 */

#ifndef INCLUDE_SETTINGS_H_
#define INCLUDE_SETTINGS_H_

namespace httpserver {

#define USE_ATOMIC
#define USE_CACHE
#define DEFAULT_CACHE_SIZE 100

#define MAX_HEADER_SIZE 512
#define MAX_FILE_SIZE 10240
#define MAX_RESPONSE_SIZE (MAX_HEADER_SIZE+MAX_FILE_SIZE)
#define DEFAULT_RECV_SIZE MAX_RESPONSE_SIZE

} //end of namespace

#endif /* INCLUDE_SETTINGS_H_ */
