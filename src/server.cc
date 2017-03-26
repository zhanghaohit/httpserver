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

using namespace httpserver;
using std::string;
using std::cout;
using std::endl;

aeEventLoop* el;

void ProcessTcpClientHandle (aeEventLoop *el, int fd, void *data, int mask) {
	ClientSocket* cs = static_cast<ClientSocket*>(data);
	LOG(LOG_WARNING, "client socket = %d", cs->GetFD());
	string r = cs->Recv();
	if (r.length() == 0) {
		LOG(LOG_WARNING, "remote client has close the socket");
		aeDeleteFileEvent(el, cs->GetFD(), AE_READABLE);
		delete cs;
	} else {
		LOG(LOG_WARNING, "received = %s", r.c_str());
		string to_send = "hello from server";
		assert(to_send.length() == cs->Send(to_send.c_str(), to_send.length()));
	}
}

void AcceptTcpClientHandle (aeEventLoop *el, int fd, void *data, int mask) {
	ServerSocket* ss = static_cast<ServerSocket*>(data);
	ClientSocket* cs = ss->Accept();

	if (aeCreateFileEvent(el, cs->GetFD(), AE_READABLE, ProcessTcpClientHandle, cs) == AE_ERROR) {
		LOG (LOG_FATAL, "cannot create file event");
		return;
	}
}

int main () {
	std::cout << "hello server" << std::endl;

	ServerSocket ss (12345);
	el = aeCreateEventLoop(1024);

	if (aeCreateFileEvent(el, ss.GetFD(), AE_READABLE, AcceptTcpClientHandle, &ss) == AE_ERROR) {
		LOG (LOG_FATAL, "cannot create file event");
		return -1;
	}

	startEventLoop (el);

	return 0;
}


