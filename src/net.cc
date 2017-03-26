/*
 * net.cc
 *
 *  Created on: 25 Mar 2017
 *      Author: zhang hao
 */

#include <sys/types.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>
#include <arpa/inet.h>
#include <string.h>
#include "net.h"
#include "log.h"

namespace httpserver {
ServerSocket::ServerSocket(int port, const char* bind_addr, int backlog) {
	int rv;
	char cport[6]; //max 65535
	addrinfo hints, *servinfo, *p;

	snprintf(cport, 6, "%d", port);
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE; // No effect if bindaddr != nullptr

	if ((rv = getaddrinfo(bind_addr, cport, &hints, &servinfo)) != 0) {
		LOG(LOG_WARNING, "getaddrinfo failed: %s", gai_strerror(rv));
		return;
	}

	for (p = servinfo; p != nullptr; p = p->ai_next) {
		if ((fd_ = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
			continue;

		if (bind(fd_, p->ai_addr, p->ai_addrlen) == -1) {
			LOG(LOG_FATAL, "bind: %s", strerror(errno));
			close (fd_);
			goto error;
		}

		if (listen(fd_, backlog) == -1) {
			LOG(LOG_FATAL, "listen: %s", strerror(errno));
			close (fd_);
			goto error;
		}
		goto end;
	}
	if (p == nullptr) {
		LOG(LOG_FATAL, "unable to bind socket");
		goto error;
	}

	error: fd_ = -1;
	end: freeaddrinfo(servinfo);
}

ClientSocket* ServerSocket::Accept() {
	sockaddr_storage sa;
	socklen_t salen = sizeof(sa);

	int max_ip_len = 46;
	char cip[max_ip_len];
	int cfd, cport;

	while (true) {
		cfd = accept(fd_, (sockaddr*) &sa, &salen);
		if (cfd == -1) {
			if (errno == EINTR)
				continue;
			else {
				LOG(LOG_WARNING, "accept: %s", strerror(errno));
				return nullptr;
			}
		}
		break;
	}

	if (sa.ss_family == AF_INET) {
		sockaddr_in* s = (sockaddr_in*) &sa;
		inet_ntop(AF_INET, (void*) &(s->sin_addr), cip, max_ip_len);
		cport = ntohs(s->sin_port);
	} else {
		LOG(LOG_WARNING, "not supported IPV6");
		return nullptr;
	}

	LOG(LOG_DEBUG, "accept %s:%d (socket = %d)", cip, cport, cfd);
	return new ClientSocket (string(cip), cport, cfd);
}

ClientSocket::ClientSocket(const string& ip, int port) :
		ip_(ip), port_(port) {
	int rv;
	char portstr[6]; // strlen("65535") + 1;
	addrinfo hints, *servinfo, *bservinfo, *p, *b;

	snprintf(portstr, sizeof(portstr), "%d", port);
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	if ((rv = getaddrinfo(ip.c_str(), portstr, &hints, &servinfo)) != 0) {
		LOG(LOG_FATAL, "%s", gai_strerror(rv));
		return;
	}
	for (p = servinfo; p != NULL; p = p->ai_next) {
		/* Try to create the socket and to connect it.
		 * If we fail in the socket() call, or on connect(), we retry with
		 * the next entry in servinfo. */
		if ((fd_ = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
			continue;

		if (connect(fd_, p->ai_addr, p->ai_addrlen) == -1) {
			/* If the socket is non-blocking, it is ok for connect() to
			 * return an EINPROGRESS error here. */
			if (errno == EINPROGRESS)
				goto end;
			close(fd_);
			fd_ = -1;
			continue;
		}

		/* If we ended an iteration of the for loop without errors, we
		 * have a connected socket. Let's return to the caller. */
		goto end;
	}
	if (p == nullptr)
		LOG(LOG_WARNING, "creating socket: %s", strerror(errno));

	error: if (fd_ != -1) {
		close(fd_);
		fd_ = -1;
	}

	end: freeaddrinfo(servinfo);
}

int ClientSocket::Send(const char* buf, int size) {
	int nwritten, totlen = 0;
	const char* copy = buf;
	while (totlen != size) {
		nwritten = write(fd_, buf, size - totlen);
		if (nwritten == 0)
			return totlen;
		if (nwritten == -1)
			return -1;
		totlen += nwritten;
		buf += nwritten;
	}

	LOG(LOG_DEBUG, "send %s (%d)", copy, size);

	return totlen;
}

string ClientSocket::Recv() {
	int nread;
	int count = 2048;
	char buf[count];

	nread = read(fd_, buf, count);
	if (nread == 0) {
		LOG(LOG_WARNING, "socket has been closed");
	}
	if (nread == -1) {
		LOG(LOG_FATAL, "read socket %d failed: %s (%d)", fd_, strerror(errno), errno);
	}
	return string(buf, nread);
}
} //end namespace

