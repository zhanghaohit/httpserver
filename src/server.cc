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
#include "util.h"

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
	HttpServer* ser = static_cast<HttpServer*>(data);
	ClientSocket* cs = ser->GetServreSocket().Accept();

	if (!cs) {
	  LOG(LOG_FATAL, "cannot accept client");
	  return;
	}

	LOG(LOG_INFO, "Accept client = %d", cs->GetFD());

	//if (el->CreateFileEvent(cs->GetFD(), READABLE, ProcessTcpClientHandle, cs) == ST_ERROR) {
	if (ser->DispatchClientSocket(cs) == ST_ERROR) {
		LOG (LOG_FATAL, "dispatch client socket failed");
		return;
	}
}

void StartEventLoopThread(EventLoop* el) {
  el->Start();
}

HttpServer::HttpServer(int port, const string& bind_addr, int backlog)
    : ss_(port, bind_addr, backlog) {}

int HttpServer::Start(int threads_num) {
  int st = ss_.Listen();
  if (st == ST_ERROR) return st;

  threads_num_ = threads_num;
  el_ = new EventLoop*[threads_num_];
  if (threads_num_ > 1) ethreads_ = new std::thread*[threads_num_-1];
  for (int i = 0; i < threads_num_; i++) {
    el_[i] = new EventLoop(el_size_);
  }

  if (el_[0]->CreateFileEvent(ss_.GetFD(), READABLE, AcceptTcpClientHandle, this) == ST_ERROR) {
    LOG(LOG_FATAL, "cannot create file event");
    return ST_ERROR;
  }

  for (int i = 1; i < threads_num_; i++) {
    ethreads_[i-1] = new std::thread(StartEventLoopThread, el_[i]);
  }
  el_[0]->Start();
  return st;
}

HttpServer::~HttpServer() {
  if (el_) {
    delete el_[0];
    for (int i = 1; i < threads_num_; i++) {
      delete el_[i];
      delete ethreads_[i];
    }
  }
}

//TODO: load balance
int HttpServer::DispatchClientSocket(ClientSocket* cs) {
  int hash = cs->GetFD() % threads_num_;
  EventLoop* el = el_[hash];
  if (el->CreateFileEvent(cs->GetFD(), READABLE, ProcessTcpClientHandle, cs) == ST_ERROR) {
    LOG (LOG_FATAL, "cannot create file event");
    return ST_ERROR;
  }
  return ST_SUCCESS;
}

} //end of namespace
