/*
 * server.cc
 *
 *  Created on: 25 Mar 2017
 *      Author: zhang hao
 */

#include <iostream>
#include "net.h"

using namespace httpserver;
using std::string;
using std::cout;
using std::endl;

int main () {
	std::cout << "hello server" << std::endl;

	ServerSocket ss (12345);

	ClientSocket* cs = ss.Accept();

	string r = cs->Recv();
	cout << "received: " << r << endl;

	string to_reply = "hello from server";
	cs->Send(to_reply.c_str(), to_reply.size());

	return 0;
}


