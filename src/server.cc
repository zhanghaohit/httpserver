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
	  el->DeleteFileEvent(fd, READABLE);
	} else {
	  request.Respond(cs);
	  if (!request.KeepAlive()) {
	    LOG(LOG_WARNING, "not keep alive, delete socket");
	    delete cs;
	    el->DeleteFileEvent(fd, READABLE);
	  }
	}
}

void AcceptTcpClientHandle (EventLoop *el, int fd, void *data, int mask) {
	ServerSocket* ss = static_cast<ServerSocket*>(data);
	ClientSocket* cs = ss->Accept();

	if (!cs) {
	  LOG(LOG_FATAL, "cannot accept client");
	  return;
	}

	LOG(LOG_WARNING, "Accept client = %d", cs->GetFD());

	if (el->CreateFileEvent(cs->GetFD(), READABLE, ProcessTcpClientHandle, cs) == ST_ERROR) {
		LOG (LOG_FATAL, "cannot create file event");
		return;
	}
}

HttpServer::HttpServer(int port, const string& bind_addr, int backlog)
    : ss_(port, bind_addr, backlog), el_() {}
} //end of namespace

int HttpServer::Start() {
  int st = ss_.Listen();

  if (el_.CreateFileEvent(ss_.GetFD(), READABLE, AcceptTcpClientHandle, &ss_) == ST_ERROR) {
    LOG(LOG_FATAL, "cannot create file event");
    return ST_ERROR;
  }
  el_.Start();
  return st;
}
