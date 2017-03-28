/*
 * net.h
 *
 *  Created on: 25 Mar 2017
 *      Author: zhang hao
 */

#ifndef INCLUDE_NET_H_
#define INCLUDE_NET_H_

#include <string>
#include <iostream>
#include <sstream>
#include <sys/socket.h>
#include <unistd.h>
#include "event.h"

using std::string;
using std::stringstream;

namespace httpserver {

#define TCP_BACKLOG 511
#define DEFAULT_RECV_SIZE 2048

#define ST_SUCCESS 0
#define ST_ERROR -1
#define ST_CLOSED 1
#define ST_INPROCESS 2


class Socket {
public:
	explicit Socket (int fd): fd_(fd) {};
	Socket (const string& ip, int port): ip_(ip), port_(port) {};
	Socket (const string& ip, int port, int fd): ip_(ip), port_(port), fd_(fd) {};
	~Socket () {
	  if (fd_ > 0) {
	    close(fd_);
	  }
	}

	inline int GetFD() const noexcept {
		return fd_;
	}

	inline void SetFD (int fd) noexcept {
		fd_ = fd;
	}

protected:
	int fd_ = -1;
  string ip_;
  int port_ = 0;
};

class ClientSocket: public Socket {
public:
	ClientSocket (const string& ip, int port);
	ClientSocket (const string& ip, int port, int fd):
		Socket(ip, port, fd) {};

	int Connect ();
	int Send (const void* buf, int size);
	int Recv (void* buf, int size = DEFAULT_RECV_SIZE);
	string Recv (int size = DEFAULT_RECV_SIZE);
	int Recv (stringstream& ss, int size = DEFAULT_RECV_SIZE);
};

class ServerSocket: public Socket {
public:
	explicit ServerSocket (int port, const string& bind_addr = "", int backlog = TCP_BACKLOG);

	int Listen ();
	ClientSocket* Accept ();

private:
	int backlog_;
};
} //end namespace

#endif /* INCLUDE_NET_H_ */
