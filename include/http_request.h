/*
 * http_request.h
 *
 *  Created on: Mar 27, 2017
 *      Author: zhanghao
 */

#ifndef INCLUDE_HTTP_REQUEST_H_
#define INCLUDE_HTTP_REQUEST_H_

#include <unordered_map>
#include <iostream>
#include <sstream>
#include <string>
#include "net.h"

using std::string;
using std::unordered_map;

namespace httpserver {

class HttpRequest {
 public:
  HttpRequest() {}

  int ReadAndParse(ClientSocket* socket);
  int Respond(ClientSocket* socket);

  inline bool KeepAlive() { return keep_alive_; }

 private:
  string method_;
  string uri_;
  string http_version_;

  bool keep_alive_ = false;
  unordered_map<string, string> params_;
  unordered_map<string, string> headers_;

  /* status_: used to transmit server's error status, such as
   o  202 OK
   o  404 Not Found
   and so on */
  string status_ = "200 OK";
};
}  //end of namespace

#endif /* INCLUDE_HTTP_REQUEST_H_ */
