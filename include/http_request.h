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

#define MAX_HEADER_SIZE 512
#define MAX_FILE_SIZE 2048
#define MAX_SIZE_BITS 4

const string kContentLen = "Content-Length: ";
const string kHttpVersion = "HTTP/1.1 ";
const string kConnection = "Connection: ";
const string kOk = "202 OK\n";
const string kNotFound = "404 Not Found\n";
//const string kKeepAlive = "Connection: keep-alive\n";
//const string kClosed = "Connection: closed\n";
const string kOtherHeaders = "Connection: keep-alive\nContent-Type: text/html\nServer: Simple Http Server\n";

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
  unordered_map<string, string> headers_;

  /* status_: used to transmit server's error status, such as
     202 OK
     404 Not Found
     and so on */
  string status_ = kOk;
};
}  //end of namespace

#endif /* INCLUDE_HTTP_REQUEST_H_ */
