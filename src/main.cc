/*
 * main.cc
 *
 *  Created on: Mar 27, 2017
 *      Author: zhanghao
 */

#include <cstring>
#include "server.h"
using namespace httpserver;

#define DEFAULT_PORT 12345

int main (int argc, char* argv[]) {
  int port = DEFAULT_PORT;
  int threads = 3; //number of threads used by the server
  int elsize = 10000; //event loop size (max concurrent connections supported)
  std::string root_dir = "./"; //root directory of the website
  std::string bind_addr = "";

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
    } else if (strcmp(argv[i], "--bind_addr") == 0) {
      bind_addr = string(argv[++i]);
    } else if (strcmp(argv[i], "--help") == 0) {
      printf("\nUsage:\n./server\n"
          "[--port port (default: %d)]\n"
          "[--bind_addr (default: )]\n"
          "[--threads threads (default: %d)]\n"
          "[--connections supported_max_connections (default: %d)]\n"
          "[--root_dir root_dir (default: %s]\n", port, threads, elsize, root_dir.c_str());
      return -1;
    } else {
      fprintf(stderr, "Unrecognized option %s for benchmark\n", argv[i]);
    }
  }

  printf("Configuration: port: %d, bind_addr: %s, threads: %d, connections: %d, root_dir: %s\n",
      port, bind_addr.c_str(), threads, elsize, root_dir.c_str());

  HttpServer server (port, bind_addr); //create the HttpServer
  server.SetRootDir(root_dir); //set root directory
  server.SetEventLoopSize(elsize); //set the max concurrent connections to support

  if (server.Start(threads) != ST_SUCCESS) { //start the http server
    printf("start httpserver error\n");
  }
  return 0;
}
