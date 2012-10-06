#include "network.h"

Network::Network(string server_ip, string server_port, int net_type) :
					server_ip_(server_ip), server_port_(server_port), net_type_(net_type),
					send_bytes_(0), recv_bytes_(0)
{

}

Network::~Network()
{

}

int Network::Startup()
{
	zmq_context_ = zmq_init(1);
	if(zmq_context_ == NULL)
		return -1;

	zmq_socket_ = zmq_socket(zmq_context_, net_type_);
	if(zmq_socket_ == NULL)
		return -2;

	return 1;
	
}

int Network::Connect()
{
	string remote_addr("tcp://");
	remote_addr += server_ip_;
	remote_addr += ":";
	remote_addr += server_port_;

	if(zmq_connect(zmq_socket_, remote_addr.c_str()) != 0)
	{
		return -1;
	}

	return 1;
}

int Network::Bind()
{
	string local_addr("tcp://");
	local_addr += server_ip_;
	local_addr += ":";
	local_addr += server_port_;

	if(zmq_bind(zmq_socket_, local_addr.c_str()) != 0)
	{
		return -1;
	}

	return 1;
}

int Network::Send(char *send_buf, Uint4 len)
{
	zmq_msg_t msg;

	send_bytes_ += len;

	if(zmq_msg_init_size(&msg, len) != 0)
		return -1;

	memcpy(zmq_msg_data(&msg), send_buf, len);

	if(zmq_send(zmq_socket_, &msg, 0) != 0)
		return -2;

	if(zmq_msg_close (&msg) != 0)
		return -3;

	return 1;
}

int Network::Recv(char *recv_buf, Uint4 &len)
{
	zmq_msg_t msg;

	if(zmq_msg_init(&msg) != 0)
		return -1;

	if(zmq_recv(zmq_socket_, &msg, 0) != 0)
		return -2;

	char *p = (char*)zmq_msg_data(&msg);
	Uint4 msg_len = zmq_msg_size(&msg);

	memcpy(recv_buf, p, msg_len);
	len = msg_len;

	if(zmq_msg_close (&msg) != 0)
		return -3;

	recv_bytes_ += len;

	return 1;
}

int Network::Close()
{
	if(zmq_close(zmq_socket_) != 0)
		return -1;

	if(zmq_term(zmq_context_) != 0)
		return -2;

	return 1;
}

void Network::PrintFlow()
{
	printf("tatal send: %u\n", send_bytes_);
	printf("total recv: %u\n", recv_bytes_);
	printf("tatal flow: %u\n", send_bytes_ + recv_bytes_);
}


