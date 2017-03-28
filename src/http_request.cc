/*
 * http_request.cc
 *
 *  Created on: Mar 27, 2017
 *      Author: zhanghao
 */

#include <algorithm>
#include "http_request.h"
#include "log.h"

using std::cout;
using std::endl;

namespace httpserver {

int HttpRequest::ReadAndParse (ClientSocket* socket) {
  stringstream ss;
  int nread = socket->Recv(ss);
  if (nread <= 0) { //remote has close the socket
    return ST_CLOSED;
  }

  string line;
  int start = 0, end = 0;
  getline(ss, line);
  if (line.length() > 0) { //parse the request line: GET /hello.htm HTTP/1.1
    //parse method
    if ((end = line.find(' ')) != string::npos) {
      method_ = line.substr(start, end-start);

      //parse request URI
      start = end+1;
      if ((end = line.find(' ', start)) != string::npos) {
        uri_ = line.substr(start, end-start);
      }

      //parse http version
      start = end+1;
      if ((end = line.find('/', start)) != string::npos) {
        http_version_ = line.substr(end+1);
      }
    }
  }

  //parse other headers
  while (getline(ss, line)) {
    start = 0;
    if ((end = line.find(':', start)) != string::npos) {
      start = end+1;
      while (start < line.length() && line[start] == ' ') start++;

      string key = line.substr(0, end);
      std::transform(key.begin(), key.end(), key.begin(), ::tolower);

      if (key.compare("connection") == 0) {
        string value = line.substr(start);
        std::transform(value.begin(), value.end(), value.begin(), ::tolower);
        if (value.find("keep-alive") != string::npos) {
          keep_alive_ = true;
        }
      }
      headers_[key] = line.substr(start);
    }
  }

  //LOG(LOG_WARNING, "method = %s, uri = %s, http_version = %s", method_.c_str(), uri_.c_str(), http_version_.c_str());
//  for (auto& it: headers_) {
//    LOG(LOG_WARNING, "%s: %s", it.first.c_str(), it.second.c_str());
//  }

  return ST_SUCCESS;
}


int HttpRequest::Respond (ClientSocket* socket) {
  stringstream ss;
  string reply = "<html><body><h1>Hello, World!</h1></body></html>";

  ss << "HTTP/1.1 " << status_ + "\n";
  ss << "Server: Simple Http Server\nContent-Type: text/html\n";
  if (headers_.count("connection")) {
    ss << "Connection: " << headers_["connection"] << "\n";
  }
  ss << "Content-Length: ";
  ss << reply.length();
  ss << "\n\n";
  ss << reply;

  int length = ss.str().length();
  if (ss.str().length() == socket->Send(ss.str().c_str(), length)) {
    return ST_SUCCESS;
  } else {
    return ST_ERROR;
  }
}

} //end of namespace

