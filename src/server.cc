/*
 * server.cc
 *
 *  Created on: 25 Mar 2017
 *      Author: zhang hao
 */

#include <iostream>
#include "net.h"
#include "event.h"
#include "log.h"
#include "http_request.h"
#include "server.h"

using namespace httpserver;
using std::string;
using std::cout;
using std::endl;

namespace httpserver {

void ProcessTcpClientHandle (EventLoop *el, int fd, void *data, int mask) {
	ClientSocket* cs = static_cast<ClientSocket*>(data);
	//LOG(LOG_WARNING, "Process client = %d", cs->GetFD());

	HttpRequest request;
	int status = request.ReadAndParse(cs);
	if (status == ST_CLOSED) {
	  delete cs;
	  el->DeleteFileEvent(fd, AE_READABLE);
	} else {
	  request.Respond(cs);
	  if (!request.KeepAlive()) {
	    LOG(LOG_WARNING, "not keep alive, delete socket");
	    delete cs;
	    el->DeleteFileEvent(fd, AE_READABLE);
	  }
	}
}

void AcceptTcpClientHandle (EventLoop *el, int fd, void *data, int mask) {
	ServerSocket* ss = static_cast<ServerSocket*>(data);
	ClientSocket* cs = ss->Accept();

	LOG(LOG_WARNING, "Accept client = %d", cs->GetFD());

	if (el->CreateFileEvent(cs->GetFD(), AE_READABLE, ProcessTcpClientHandle, cs) == ST_ERROR) {
		LOG (LOG_FATAL, "cannot create file event");
		return;
	}
}

HttpServer::HttpServer(int port, const char* bind_addr, int backlog)
    : ss_(port, bind_addr, backlog), el_(MAX_FD) {

  if (el_.CreateFileEvent(ss_.GetFD(), AE_READABLE, AcceptTcpClientHandle, &ss_) == ST_ERROR) {
    LOG(LOG_FATAL, "cannot create file event");
    return;
  }
}
} //end of namespace

