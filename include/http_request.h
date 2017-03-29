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
#include "util.h"
#include "net.h"
#include "settings.h"

using std::string;
using std::unordered_map;

namespace httpserver {

const string CRLF = "\r\n";
const string kContentLen = "Content-Length: ";
const string kHttpVersion = "HTTP/1.1 ";
const string kConnection = "Connection: ";
const string kOk = "202 OK\r\n";
const string kNotFound = "404 Not Found\r\n";
//const string kKeepAlive = "Connection: keep-alive\r\n";
//const string kClosed = "Connection: closed\r\n";
const string kContentType = "Content-Type: ";
const string kOtherHeaders = "Connection: keep-alive\r\nServer: Simple Http Server\r\n";


class HttpRequest {
 public:
  HttpRequest() {}

  int ReadAndParse(ClientSocket* socket);
  int Respond(ClientSocket* socket);

  inline bool KeepAlive() { return keep_alive_; }

 private:
  int ProcessFirstLine(char* buf, int start, int end); //line: buf[start, end] excluding \r\n
  int ProcessOneLine(char* buf, int start, int end); //line: buf[start, end] excluding \r\n
  inline void TrimSpace(const char* buf, int& start, int end) {
    while (start <= end && buf[start] == ' ') start++;
  }

  inline void TrimSpecial(const char* buf, int& start, int end) {
    while (start <= end && (buf[start] == ' ' || buf[start] == '\n' || buf[start] == '\r')) start++;
  }

  inline void TrimSpecialReverse(const char* buf, int& end, int start) {
    while (end >= start && (buf[end] == ' ' || buf[end] == '\n' || buf[end] == '\r')) end--;
  }

  inline int FindChar(const char* buf, int start, int end, char c) { //find within [start, end]
    while (start <= end) {
      if (buf[start] == c) break;
      start++;
    }
    return start;
  }

  //find the position of the specified character, and tolower the character before this character
  inline int FindCharToLower(char* buf, int start, int end, char c) { //find within [start, end]
    while (start <= end) {
      if (buf[start] == c) {
        break;
      } else {
        buf[start] = std::tolower(buf[start]);
      }
      start++;
    }
    return start;
  }

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
