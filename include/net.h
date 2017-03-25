/*
 * net.h
 *
 *  Created on: 25 Mar 2017
 *      Author: zhang hao
 */

#ifndef INCLUDE_NET_H_
#define INCLUDE_NET_H_

#include <string>
#include <sys/socket.h>

using std::string;

namespace httpserver {

#define TCP_BACKLOG 511


class Socket {
public:
	Socket (int fd = -1): fd_(fd) {};
	~Socket () { close(fd_); }

	inline int GetFD () {
		return fd_;
	}

	inline void SetFD (int fd) {
		fd_ = fd;
	}

protected:
	int fd_ = -1;
};

class ClientSocket: public Socket {
public:
	ClientSocket (const string& ip, int port);
	ClientSocket (const string& ip, int port, int fd):
		Socket (fd), ip_(ip), port_(port) {};

	int Send (const char* buf, int size);
	string Recv ();

private:
	string ip_;
	int port_;
};

class ServerSocket: public Socket {
public:
	ServerSocket (int port, const char* bind_addr = nullptr, int backlog = TCP_BACKLOG);

	ClientSocket* Accept ();
private:

};
} //end namespace

#endif /* INCLUDE_NET_H_ */
