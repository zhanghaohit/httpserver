/*
 * main.cc
 *
 *  Created on: Mar 27, 2017
 *      Author: zhanghao
 */

#include <cstring>
#include "server.h"
#include "log.h"
using namespace httpserver;

#define DEFAULT_PORT 12345

int main (int argc, char* argv[]) {
  int port = DEFAULT_PORT;

  //the first argument should be the program name
  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "--port") == 0) {
      port = atoi(argv[++i]);
    } else {
      fprintf(stderr, "Unrecognized option %s for benchmark\n", argv[i]);
    }
  }

  LOG(LOG_WARNING, "port: %d", port);


  HttpServer server (port);
  if (server.Start() != ST_SUCCESS) {
    LOG(LOG_WARNING, "start httpserver error");
  }
  return 0;
}

