#include "file_backup.h"

static char recv_buf[kNetBufSize];
static char send_buf[kNetBufSize];

FileBackup::FileBackup(string inc_file_path) : inc_file_path_(inc_file_path)
{

}

FileBackup::~FileBackup()
{

}

int FileBackup::BackupIncFile(Network &net)
{
	Uint4 msg_len = 0;

	send_buf[0] = INC_START_REQ;
	if(net.Send(send_buf, 1) < 0)
		return -1;

	if(net.Recv(recv_buf, msg_len) < 0)
		return -2;

	if(recv_buf[0] != INC_START_REP || recv_buf[1] == 0)
		return -3;

	FILE *fp = fopen(inc_file_path_.c_str(), "rb");
	if(fp == NULL)
	{
		return -4;
	}

	Uint4 ret = 0;
	while(true)
	{
		send_buf[0] = INC_DATA_REQ;
		ret = fread(send_buf+1, 1, kNetBufSize-1, fp);
		if(ret == 0)
			break;
			
		if(net.Send(send_buf, ret+1) < 0)
			return -5;

		if(net.Recv(recv_buf, msg_len) < 0)
			return -6;
		if(recv_buf[0] != INC_DATA_REP)
			return -7;

		if(ret != kNetBufSize -1)
			break;
		
	}
	fclose(fp);

	send_buf[0] = INC_END_REQ;
	if(net.Send(send_buf, 1) < 0)
		return -8;

	if(net.Recv(recv_buf, msg_len) < 0)
		return -9;

	if(recv_buf[0] != INC_END_REP || recv_buf[1] != 1)
	{
		return -10;
	}

	return 1;
		
}
