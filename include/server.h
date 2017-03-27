/*
 * server.h
 *
 *  Created on: Mar 27, 2017
 *      Author: zhanghao
 */

#ifndef INCLUDE_SERVER_H_
#define INCLUDE_SERVER_H_

#include "net.h"

#define MAX_FD 1024

namespace httpserver {
class HttpServer {
 public:
  explicit HttpServer(int port, const char* bind_addr = nullptr, int backlog = TCP_BACKLOG);

  inline void Start() {
    el_.Start();
  }

  inline void Stop() {
    el_.Stop();
  }

 private:
  ServerSocket ss_;
  EventLoop el_;
};
} //end of namespace


#endif /* INCLUDE_SERVER_H_ */
