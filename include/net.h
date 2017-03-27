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


class Socket {
public:
	Socket (int fd = -1): fd_(fd) {};
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
	int fd_;
};

class ClientSocket: public Socket {
public:
	ClientSocket (const string& ip, int port);
	ClientSocket (const string& ip, int port, int fd):
		Socket (fd), ip_(ip), port_(port) {};

	int Send (const char* buf, int size);
	int Recv (char* buf, int size = DEFAULT_RECV_SIZE);
	string Recv (int size = DEFAULT_RECV_SIZE);
	int Recv (stringstream& ss, int size = DEFAULT_RECV_SIZE);

private:
	string ip_;
	int port_;
};

class ServerSocket: public Socket {
public:
	explicit ServerSocket (int port, const char* bind_addr = nullptr, int backlog = TCP_BACKLOG);

	ClientSocket* Accept ();
private:

};
} //end namespace

#endif /* INCLUDE_NET_H_ */
