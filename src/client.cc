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

	string to_send = "hello from client";
	assert(to_send.length() == cs.Send(to_send.c_str(), to_send.length()));

	string r = cs.Recv();
	cout << "received: " << r << endl;

	return 0;
}

