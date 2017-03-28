/*
 * server.h
 *
 *  Created on: Mar 27, 2017
 *      Author: zhanghao
 */

#ifndef INCLUDE_SERVER_H_
#define INCLUDE_SERVER_H_

#include "net.h"

namespace httpserver {
class HttpServer {
 public:
  explicit HttpServer(int port, const string& bind_addr = "", int backlog = TCP_BACKLOG);

  int Start();

  inline void Stop() {
    el_.Stop();
  }

 private:
  ServerSocket ss_;
  EventLoop el_;
};
} //end of namespace


#endif /* INCLUDE_SERVER_H_ */
