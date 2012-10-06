#include "chunk_dedup.h"

static char recv_buf[kNetBufSize];
static char send_buf[kNetBufSize];
static char file_buf[kFileBufSize];


ChunkDedup::ChunkDedup(string chunk_file_path, string inc_file_path) :
						chunk_file_path_(chunk_file_path), inc_file_path_(inc_file_path)
{

}

ChunkDedup::~ ChunkDedup()
{

}

int ChunkDedup::DedupChunk(vector <ChunkInfo> &partitions, Network &net)
{
	Uint4 chunk_count = partitions.size();
	Uint4 msg_len = 0;

	send_buf[0] = CHUNK_START_REQ;
	memcpy(send_buf+1, &chunk_count, 4);
	Uint4 path_len = chunk_file_path_.size();
	memcpy(send_buf+5, chunk_file_path_.c_str(), path_len+1);
	if(net.Send(send_buf, path_len+1+5) < 0)
		return -1;

	if(net.Recv(recv_buf, msg_len) < 0)
	{
		return -2;
	}
	if(recv_buf[0] != CHUNK_START_REP)
	{
		return -3;
	}

	for(Uint4 i = 0; i < chunk_count; i++)
	{
		send_buf[0] = CHUNK_INFO_REQ;
		memcpy(send_buf+1, &partitions[i].len, 2);
		memcpy(send_buf+3, &partitions[i].sha1, 20);

		if(net.Send(send_buf, 23) < 0)
		{
			return -4;
		}

		if(net.Recv(recv_buf, msg_len) < 0 || recv_buf[0] != CHUNK_INFO_REP)
		{
			return -5;
		}
		
	}

	send_buf[0] = CHUNK_END_REQ;
	if(net.Send(send_buf, 1) < 0)
	{
		return -6;
	}
	if(net.Recv(recv_buf, msg_len) < 0)
	{
		return -7;
	}
	if(recv_buf[0] != CHUNK_END_REP || msg_len != chunk_count+1)
	{
		return -8;
	}

	FILE *chunk_file_fd = fopen(chunk_file_path_.c_str(), "rb");
	if(chunk_file_fd == NULL)
		return -9;
	FILE *inc_file_fd = fopen(inc_file_path_.c_str(), "wb");
	if(inc_file_fd == NULL)
		return -10;

	
	for(Uint4 i = 0; i < chunk_count; i++)
	{
		if(recv_buf[1+i] == 0)
		{
			#ifdef DEBUG_
			printf("%d ", partitions[i].len);
			#endif
			fseek(chunk_file_fd, partitions[i].offset, SEEK_SET);
			fread(file_buf, 1, partitions[i].len, chunk_file_fd);
			fwrite(file_buf, 1, partitions[i].len, inc_file_fd);
		}
	}

	#ifdef DEBUG_
	printf("\n");
	#endif

	fclose(chunk_file_fd);
	fclose(inc_file_fd);

	return 1;
}

