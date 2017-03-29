/*
 * client.cc
 *
 *  Created on: 25 Mar 2017
 *      Author: zhang hao
 */

#include <iostream>
#include <cassert>
#include <thread>
#include <cstring>
#include <atomic>
#include "net.h"
#include "log.h"
#include "util.h"

using namespace httpserver;
using std::string;
using std::cout;
using std::endl;
using std::atomic;

#define SOCKETS_PER_THREAD 100
#define DEFAULT_PORT 12345

const string kToSend = " GET  /index.html  HTTP/1.1\r\n"
  " Host:  localhost:12345\r\n"
    "User-Agent: Mozilla/5.0 (X11; Ubuntu; Linux x86_64; rv:44.0) Gecko/20100101 Firefox/44.0\r\n"
    "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\n"
    "Accept-Language: en-US,en;q=0.5\n"
    "Accept-Encoding: gzip, deflate\r\n"
    "Connection: keep-alive\r\n";

atomic<unsigned long> sent (0);
atomic<unsigned long> recved (0);

void request(EventLoop *el, int fd, void *data, int mask) {
  ClientSocket* cs = static_cast<ClientSocket*>(data);
  string r = cs->Recv();
  recved++;
  if (r.size() == 0) {
    LOG(LOG_WARNING, "received from server failed");
    el->DeleteFileEvent(fd, READABLE);
    return;
  }
  //LOG(LOG_WARNING, "Received = %s, size = %d", r.c_str(), r.size());
  if(kToSend.length() != cs->Send(kToSend.c_str(), kToSend.length())) {
    LOG(LOG_WARNING, "request failed");
  }
  sent++;
}

void print_stats() {
  while (true) {
    long start = get_time();
    unsigned long s1 = sent;
    unsigned long r1 = recved;
    sleep(1);
    unsigned long s2 = sent;
    unsigned long r2 = recved;
    long end = get_time();
    double duration = (end - start) * 1.0 / 1000 / 1000 / 1000;
    LOG(LOG_WARNING, "throughput:\nsend: %lf ops/s, receive: %lf ops/s",
        (s2 - s1) / duration, (r2 - r1) / duration);
  }
}

void start_client (EventLoop* el) {
  el->Start();
}

int main (int argc, char* argv[]) {
  int num_sockets = 100;
  int port = DEFAULT_PORT;
  string ip = "localhost";
  int elsize = 10000;

  //the first argument should be the program name
  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "--port") == 0) {
      port = atoi(argv[++i]);
    } else if (strcmp(argv[i], "--ip") == 0) {
      ip = argv[++i];
    } else if (strcmp(argv[i], "--sockets") == 0) {
      num_sockets = atoi(argv[++i]);
    } else if (strcmp(argv[i], "--connections") == 0) {
      elsize = atoi(argv[++i]);
    } else if (strcmp(argv[i], "--help") == 0) {
      printf("\nUsage:\n./client\n"
          "[--port port (default: %d)]\n"
          "[--connections supported_max_connections (default: %d)]\n"
          "[--sockets num_sockets (default: %d]\n", DEFAULT_PORT, elsize, num_sockets);
      return -1;
    } else {
      fprintf(stderr, "Unrecognized option %s for benchmark\n", argv[i]);
    }
  }

  LOG(LOG_WARNING, "ip = %s, port: %d, num_sockets = %d", ip.c_str(), port, num_sockets);

  EventLoop* el = new EventLoop(10000);

  ClientSocket* cs[num_sockets];
  for (int i = 0; i < num_sockets; i++) {
    cs[i] = new ClientSocket(ip, port);
    if (cs[i]->Connect() != ST_SUCCESS) {
      LOG(LOG_WARNING, "cannot connect to the server");
      return -1;
    } else {
      el->CreateFileEvent(cs[i]->GetFD(), READABLE, request, cs[i]);
    }
  }

  std::thread client_thread (start_client, el); //start the client thread

  for (int i = 0; i < num_sockets; i++) { //send initial requests
    if(kToSend.length() != cs[i]->Send(kToSend.c_str(), kToSend.length())) {
      LOG(LOG_WARNING, "request failed");
    }
    sent++;
  }

  std::thread t (print_stats);

  client_thread.join();
	return 0;
}

