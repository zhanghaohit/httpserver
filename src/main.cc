/*
 * main.cc
 *
 *  Created on: Mar 27, 2017
 *      Author: zhanghao
 */

#include "server.h"
#include "log.h"
using namespace httpserver;

#define DEFAULT_PORT 12345

int main (int argc, char* argv[]) {
  int port = DEFAULT_PORT;
  if (argc < 2) {
    LOG(LOG_WARNING, "using default port: %d", port);
  } else {
    port = atoi(argv[1]);
    LOG(LOG_WARNING, "using port: %d", port);
  }

  HttpServer server (port);
  server.Start();
  return 0;
}

