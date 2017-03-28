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

string to_send = "GET /index.html HTTP/1.1\n"
  "Host: localhost:12345\n"
    "User-Agent: Mozilla/5.0 (X11; Ubuntu; Linux x86_64; rv:44.0) Gecko/20100101 Firefox/44.0\n"
    "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\n"
    "Accept-Language: en-US,en;q=0.5\n"
    "Accept-Encoding: gzip, deflate\n"
    "Connection: keep-alive\n";

atomic<unsigned long> sent (0);
atomic<unsigned long> recved (0);
int num_requests = 100000;

void request(EventLoop *el, int fd, void *data, int mask) {
  ClientSocket* cs = static_cast<ClientSocket*>(data);
  string r = cs->Recv(1024);
  recved++;
  unsigned long rc = recved;
//  if (rc > num_requests) {
//    el->Stop();
//  }
  if (r.size() == 0) {
    LOG(LOG_WARNING, "received from server failed");
    return;
  }
  //LOG(LOG_WARNING, "Received = %s, size = %d", r.c_str(), r.size());
  unsigned long s = sent;
  sleep(1);
  if(to_send.length() != cs->Send(to_send.c_str(), to_send.length())) {
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
    LOG(LOG_WARNING, "throughput:\nsend: %lf ops/s (%lu - %lu), receive: %lf ops/s (%lu - %lu) (duration = %f s)",
        (s2 - s1) / duration, s2, s1, (r2 - r1) / duration, r2, r1, duration);
  }
}

void start_client (EventLoop* el) {
  el->Start();
}

int main (int argc, char* argv[]) {
  int num_sockets = 10000;
  int port = DEFAULT_PORT;
  string ip = "localhost";

  //the first argument should be the program name
  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "--port") == 0) {
      port = atoi(argv[++i]);
    } else if (strcmp(argv[i], "--ip") == 0) {
      ip = argv[++i];
    } else if (strcmp(argv[i], "--sockets") == 0) {
      num_sockets = atoi(argv[++i]);
    } else if (strcmp(argv[i], "--requests") == 0) {
      num_requests = atoi(argv[++i]);
    } else {
      fprintf(stderr, "Unrecognized option %s for benchmark\n", argv[i]);
    }
  }

  LOG(LOG_WARNING, "ip = %s, port: %d, num_sockets = %d", ip.c_str(), port, num_sockets);

	EventLoop el;
  ClientSocket* cs[num_sockets];
  for (int i = 0; i < num_sockets; i++) {
    cs[i] = new ClientSocket(ip, port);
    if (cs[i]->Connect() != ST_SUCCESS) {
      LOG(LOG_WARNING, "cannot connect to the server");
      return -1;
    } else {
      el.CreateFileEvent(cs[i]->GetFD(), READABLE, request, cs[i]);
    }
  }

  std::thread client_thread (start_client, &el);

  for (int i = 0; i < num_sockets; i++) {
    if(to_send.length() != cs[i]->Send(to_send.c_str(), to_send.length())) {
      LOG(LOG_WARNING, "request failed");
    }
    sent++;
  }

  std::thread t (print_stats);

  client_thread.join();
  //el.Start();
	return 0;
}

