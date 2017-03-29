/*
 * main.cc
 *
 *  Created on: Mar 27, 2017
 *      Author: zhanghao
 */

#include <cstring>
#include "server.h"
#include "log.h"
#include "resource.h"
using namespace httpserver;

#define DEFAULT_PORT 12345

int main (int argc, char* argv[]) {
  int port = DEFAULT_PORT;
  int threads = 4;
  int elsize = 10000;
  std::string root_dir = "./";

  //the first argument should be the program name
  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "--port") == 0) {
      port = atoi(argv[++i]);
    } else if (strcmp(argv[i], "--threads") == 0) {
      threads = atoi(argv[++i]);
    } else if (strcmp(argv[i], "--connections") == 0) {
      elsize = atoi(argv[++i]);
    } else if (strcmp(argv[i], "--root_dir") == 0) {
      root_dir = string(argv[++i]);
    } else if (strcmp(argv[i], "--help") == 0) {
      printf("\nUsage:\n./server\n"
          "[--port port (default: %d)]\n[--threads threads (default: %d)]\n"
          "[--connections supported_max_connections (default: %d)]\n"
          "[--root_dir root_dir (default: %s]\n", port, threads, elsize, root_dir.c_str());
      return -1;
    } else {
      fprintf(stderr, "Unrecognized option %s for benchmark\n", argv[i]);
    }
  }

  printf("Configuration:\nport: %d, threads: %d, connections: %d, root_dir: %s\n",
      port, threads, elsize, root_dir.c_str());

  HttpResource::Instance()->SetRootDir(root_dir);
  HttpServer server (port);
  server.SetEventLoopSize(elsize); //concurrent connections

  if (server.Start(threads) != ST_SUCCESS) {
    LOG(LOG_WARNING, "start httpserver error");
  }
  return 0;
}

