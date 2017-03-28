/*
 * http_request.cc
 *
 *  Created on: Mar 27, 2017
 *      Author: zhanghao
 */

#include <algorithm>
#include <string.h>
#include "http_request.h"
#include "log.h"
#include "resource.h"

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
  //string reply = "<html><body><h1>Hello, World!</h1></body></html>";
  char header[MAX_HEADER_SIZE];
  char* buf = static_cast<char*>(malloc(MAX_FILE_SIZE));
  int pos = 0;

  int size = HttpResource::Instance()->Get(uri_, buf, MAX_FILE_SIZE);;
  if (size == 0) {  //read file failed
    status_ = kNotFound;
  } else if (size > MAX_FILE_SIZE) { //file is larger than the buffer
    LOG(LOG_WARNING, "found a big file with size %d", size);
    buf = static_cast<char*>(realloc(buf, size));
    size = HttpResource::Instance()->Get(uri_, buf, size);
  }

  memcpy(header+pos, kHttpVersion.c_str(), kHttpVersion.length());
  pos += kHttpVersion.length();

  memcpy(header+pos, status_.c_str(), status_.length());
  pos += status_.length();

  memcpy(header+pos, kOtherHeaders.c_str(), kOtherHeaders.length());
  pos += kOtherHeaders.length();

  memcpy(header+pos, kContentLen.c_str(), kContentLen.length());
  pos += kContentLen.length();
  pos += sprintf(header+pos, "%d\n\n", size);

  if (pos != socket->Send(header, pos)) {
    free(buf);
    return ST_ERROR;
  }

  if (size > 0) {
    if (size != socket->Send(buf, size)) {
      free(buf);
      return ST_ERROR;
    }
  }

  free(buf);
  return ST_SUCCESS;
}

} //end of namespace

