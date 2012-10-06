#ifndef DDRBS_NETWORK_H
#define DDRBS_NETWORK_H

#include <string>
#include <zmq.h>
#include "utility.h"

using std::string;

class Network
{
public:
	Network(string server_ip, string server_port, int net_type);
	~Network();

	int Startup();
	int Connect();
	int Bind();	
	int Send(char *send_buf, Uint4 len);
	int Recv(char *recv_buf, Uint4 &len);
	int Close();
private:

private:
	string server_ip_;
	string server_port_;
	int net_type_;
	void *zmq_context_;
	void *zmq_socket_;
};

#endif 
