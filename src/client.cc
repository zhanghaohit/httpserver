/*
 * client.cc
 *
 *  Created on: 25 Mar 2017
 *      Author: zhang hao
 */

#include <iostream>
#include <cassert>
#include "net.h"

using namespace httpserver;
using std::string;
using std::cout;
using std::endl;

int main () {
	std::cout << "hello client" << std::endl;

	ClientSocket cs ("localhost", 12345);

	string to_send = "Host: localhost:12345\n"
	    "User-Agent: Mozilla/5.0 (X11; Ubuntu; Linux x86_64; rv:44.0) Gecko/20100101 Firefox/44.0\n"
	    "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\n"
	    "Accept-Language: en-US,en;q=0.5\n"
	    "Accept-Encoding: gzip, deflate\n"
	    "Connection: keep-alive\n";
	assert(to_send.length() == cs.Send(to_send.c_str(), to_send.length()));

	string r = cs.Recv(1024);
	cout << "received: " << r << endl;

	sleep(1);

	return 0;
}

