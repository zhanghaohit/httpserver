/*
 * server.h
 *
 *  Created on: Mar 27, 2017
 *      Author: zhanghao
 */

#ifndef INCLUDE_SERVER_H_
#define INCLUDE_SERVER_H_

#include <thread>
#include "net.h"

namespace httpserver {
class HttpServer {
 public:
  explicit HttpServer(int port, const string& bind_addr = "", int backlog = TCP_BACKLOG);
  ~HttpServer();

  int Start(int threads_num = 1);

  inline void Stop() {
    for (int i = 0; i < threads_num_; i++) el_[i]->Stop();
  }

  inline ServerSocket& GetServreSocket() {
    return ss_;
  }

  inline void SetEventLoopSize (int size) {
    el_size_ = size;
  }

  int DispatchClientSocket(ClientSocket* cs);

 private:
  ServerSocket ss_;
  int threads_num_ = 1;
  int el_size_ = 10000;
  std::thread** ethreads_ = nullptr;
  EventLoop** el_ = nullptr;
};
} //end of namespace


#endif /* INCLUDE_SERVER_H_ */
