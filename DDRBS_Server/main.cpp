#include <cstdio>
#include <cstring>
#include <vector>
#include <string>
#include "utility.h"
#include "network.h"
#include "chunk_db.h"

using std::vector;
using std::string;

static char recv_buf[kNetBufSize];
static char send_buf[kNetBufSize];
static char file_buf[kFileBufSize];
static Uint4 msg_len = 0;


static char backup_path[kMaxPathLen]; //这次备份选中的目录
/*记录每个备份文件的信息，包括文件的路径、
块信息在compose文件中的偏移，文件中块的数量*/
const char *kFilesInfoPath = "/root/DDRBS/bkinfo/files_info"; 
/*记录每个文件组成的所有块信息，包括块所在容器文件、
在容器文件中的偏移，块的大小*/
const char *kFilesComposePath = "/root/DDRBS/bkinfo/files_compose";
static FILE *files_info_fp = NULL;
static FILE *files_compose_fp = NULL;
static Uint4 current_file_compose_offset = 0; //当前备份文件块信息在compose文件中偏移

static char current_file_path[kMaxPathLen];//当前备份文件的路径
static Uint4 current_chunk_count = 0;//当前文件的块的数量

const char *kContainerRecordPath = "/root/DDRBS/container/record";//存放容器的信息
const char *kContainerPathPrefix = "/root/DDRBS/container/data";//容器文件的前缀，跟数字组合构成真正的路径
static Uint4 current_container_id = 0;//当前备份将要达到的容器id
static Uint4 current_container_offset = 0;//当前备份将要达到的容器偏移
static Uint4 write_container_id = 0;//将要写入数据的容器id
static Uint4 write_container_offset = 0;//将要写入数据的容器偏移
const Uint4 kContainerCapacity = 2147483647;//容器文件的容量
static FILE *write_container_fd = NULL;

const char *kCurrentIncFilePath = "/root/DDRBS/inc_file";
static FILE *current_inc_fd = NULL;

const char *kDbPath = "/root/DDRBS/db/";

static vector<ChunkInfo> file_chunks;//记录当前文件的块信息

//统计信息
Uint4 total_chunk_count = 0;
Uint4 same_chunk_count = 0;
Uint4 total_chunk_bytes = 0;
Uint4 save_chunk_bytes = 0;

static void GetContainerPath(Uint4 container_id, char *path)
{
	char tmp[8];

	sprintf(tmp, "%u", container_id);

	strcpy(path, kContainerPathPrefix);
	strcat(path, tmp);
}

static int InitContainerRecord()
{
	FILE *fp = fopen(kContainerRecordPath, "r");
	if(fp != NULL)
	{
		if(fscanf(fp, "%u %u", &current_container_id, &current_container_offset) != 2)
			return -1;
		fclose(fp);
	}
	else
	{
		current_container_id = 0;
		current_container_offset = 0;
	}

	write_container_id = current_container_id;
	write_container_offset = current_container_offset;

	return 1;
}

static int UpdateContainerRecord()
{
	FILE *fp = fopen(kContainerRecordPath, "w");
	if(fp == NULL)
		return -1;

	if(fprintf(fp, "%u %u\n", current_container_id, current_container_offset) < 0)
	{
		return -2;
	}

	fclose(fp);

	return 1;
}

static int HandleBackupStart(Network &net, ChunkDB &chunk_db)
{
	printf("backup start\n");
	
	strcpy(backup_path, recv_buf+1);
	int ret = 1;

	send_buf[0] = BACKUP_START_REP;
	send_buf[1] = 1;
	
	files_info_fp = fopen(kFilesInfoPath, "w");
	if(files_info_fp == NULL)
	{
		send_buf[1] = -1;
		ret = -1;
	}
	files_compose_fp = fopen(kFilesComposePath, "wb");
	if(files_compose_fp == NULL)
	{
		send_buf[1] = -2;
		ret = -2;
	}
	current_file_compose_offset = 0;
	
	if(chunk_db.OpenDB() < 0)
	{
		send_buf[1] = -3;
		ret = -3;
	}
	if(InitContainerRecord() < 0)
	{
		send_buf[1] = -4;
		ret = -4;
	}

	char tmp_path[kMaxPathLen];
	GetContainerPath(current_container_id, tmp_path);
	write_container_fd = fopen(tmp_path, "a");
	if(write_container_fd == NULL)
	{
		send_buf[1] = -5;
		ret = -5;
	}

	if(net.Send(send_buf, 2) < 0)
	{
		ret = -6;
	}

	total_chunk_count = 0;
	same_chunk_count = 0;
	total_chunk_bytes = 0;
	save_chunk_bytes = 0;

	return ret;
	
}

static int HandleChunkStart(Network &net)
{
	memcpy(&current_chunk_count, recv_buf+1, 4);
	strcpy(current_file_path, recv_buf+5);

	total_chunk_count += current_chunk_count;

	file_chunks.clear();

	send_buf[0] = CHUNK_START_REP;
	if(net.Send(send_buf, 1) < 0)
		return -1;

	return 1;
}
/*
static void PrintKey(string key)
{
	for(size_t i = 0; i < key.size(); i++)
	{
		printf("%02X", (unsigned char)key[i]);
	}
	printf("\n");
}*/

static int HandleChunkInfo(Network &net, ChunkDB &chunk_db)
{
	ChunkInfo chunk;
	
	memcpy(&chunk.len, recv_buf+1, 2);

	string key, value;
	key.assign(recv_buf+3, 20);

	//PrintKey(key);
	total_chunk_bytes += chunk.len;

	if(chunk_db.Get(key, &value) > 0)
	{
		chunk.is_duplication = true;
		sscanf(value.c_str(), "%u,%u", &chunk.container_id, &chunk.container_offset);
		
		file_chunks.push_back(chunk);

		same_chunk_count++;
		save_chunk_bytes += chunk.len;
	}
	else
	{
		chunk.is_duplication = false;
		
		if(kContainerCapacity - current_container_offset >= chunk.len)
		{
			chunk.container_id = current_container_id;
			chunk.container_offset = current_container_offset;

			current_container_offset += chunk.len;
		}
		else
		{
			//fclose(write_container_fd);

			current_container_id++;
			current_container_offset = 0;
			
			//char tmp_path[kMaxPathLen];
			//GetContainerPath(current_container_id, tmp_path);
			//write_container_fd = fopen(tmp_path, "a");
			//if(write_container_fd == NULL)
				//return -1;

			chunk.container_id = current_container_id;
			chunk.container_offset = current_container_offset;

			current_container_offset += chunk.len;
		}

		char tmp[128];
		sprintf(tmp, "%u,%u", chunk.container_id, chunk.container_offset);
		value.assign(tmp);
		if(chunk_db.Put(key, value) < 0)
			return -2;

		file_chunks.push_back(chunk);
		
	}

	send_buf[0] = CHUNK_INFO_REP;
	if(net.Send(send_buf, 1) < 0)
	{
		return -3;
	}

	return 1;
}

static int HandleChunkEnd(Network &net)
{
	send_buf[0] = CHUNK_END_REP;
	
	for(Uint4 i = 0; i < current_chunk_count; i++)
	{
		if(file_chunks[i].is_duplication)
		{
			send_buf[1+i] = 1;
		}
		else
		{
			send_buf[1+i] = 0;
		}
	}

	if(net.Send(send_buf, current_chunk_count+1) < 0)
		return -1;

	return 1;
}

static int HandleIncStart(Network &net)
{
	send_buf[0] = INC_START_REP;

	current_inc_fd = fopen(kCurrentIncFilePath, "wb");
	if(current_inc_fd == NULL)
		send_buf[1] = 0;
	else
		send_buf[1] = 1;

	if(net.Send(send_buf, 2) < 0)
		return -1;

	return 1;
}

static int HandleIncData(Network &net)
{
	fwrite(recv_buf+1, 1, msg_len-1, current_inc_fd);

	send_buf[0] = INC_DATA_REP;
	if(net.Send(send_buf, 1) < 0)
		return -1;

	return 1;
}

static int HandleIncEnd(Network &net)
{
	fclose(current_inc_fd);
	int ret = 0;

	send_buf[0] = INC_END_REP;
	send_buf[1] = 1;
	FILE *fp = fopen(kCurrentIncFilePath, "rb");
	if(fp == NULL)
	{
		send_buf[1] = 0;
		ret = -1;
		goto clear;
	}

	fprintf(files_info_fp, "%s %u %u\n", current_file_path, current_file_compose_offset, current_chunk_count);
	current_file_compose_offset += current_chunk_count * 10;

	#ifdef DEBUG_
	printf("count: %d\n", current_chunk_count);
	#endif

	for(Uint4 i = 0; i < current_chunk_count; i++)
	{
		fwrite(&file_chunks[i].container_id, 1, 4, files_compose_fp);
		fwrite(&file_chunks[i].container_offset, 1, 4, files_compose_fp);
		fwrite(&file_chunks[i].len, 1, 2, files_compose_fp);
		
		if(!file_chunks[i].is_duplication)
		{
			#ifdef DEBUG_
			printf("%d ", file_chunks[i].len);
			#endif
			
			if(kContainerCapacity - write_container_offset >= file_chunks[i].len)
			{
				write_container_offset += file_chunks[i].len;
			}
			else
			{
				fclose(write_container_fd);

				write_container_id++;
				write_container_offset = 0;
			
				char tmp_path[kMaxPathLen];
				GetContainerPath(write_container_id, tmp_path);
				write_container_fd = fopen(tmp_path, "a");
				if(write_container_fd == NULL)
				{
					send_buf[1] = 0;
					ret = -2;
					goto clear;
				}
				write_container_offset += file_chunks[i].len;
			}
			fread(file_buf, 1, file_chunks[i].len, fp);	
			fwrite(file_buf, 1, file_chunks[i].len, write_container_fd);
		}
	}
	send_buf[1] = 1;
	fclose(fp);

	#ifdef DEBUG_
	printf("\n");
	#endif
	
	clear:
	if(net.Send(send_buf, 2) < 0)
	{
		ret = -3;
	}

	return ret;
		
}

static int HandleBackupEnd(Network &net, ChunkDB &chunk_db)
{
	fclose(files_info_fp);
	fclose(files_compose_fp);

	chunk_db.CloseDB();

	UpdateContainerRecord();

	fclose(write_container_fd);

	send_buf[0] = BACKUP_END_REP;
	if(net.Send(send_buf, 1) < 0)
		return -1;

	printf("backup end\n");

	printf("same: %u total: %u\n", same_chunk_count, total_chunk_count);
	printf("save: %u total: %u\n\n\n", save_chunk_bytes, total_chunk_bytes);

	return 1;
}

static int HandleMsg(Network &net, ChunkDB &chunk_db)
{
	char msg_type = recv_buf[0];
	int ret = 0;

	switch(msg_type)
	{
		case BACKUP_START_REQ:
			ret = HandleBackupStart(net, chunk_db);
			break;

		case CHUNK_START_REQ:
			ret = HandleChunkStart(net);
			break;

		case CHUNK_INFO_REQ:
			ret = HandleChunkInfo(net, chunk_db);
			break;

		case CHUNK_END_REQ:
			ret = HandleChunkEnd(net);
			break;

		case INC_START_REQ:
			ret = HandleIncStart(net);
			break;

		case INC_DATA_REQ:
			ret = HandleIncData(net);
			break;

		case INC_END_REQ:
			ret = HandleIncEnd(net);
			break;

		case BACKUP_END_REQ:
			ret = HandleBackupEnd(net, chunk_db);
			break;

		default:
			ret = -111;
			break;
	}

	return ret;
}


int main()
{
	int ret = 0;

	Network net("*", "969", ZMQ_REP);
	ChunkDB chunk_db(kDbPath);
	
	if((ret = net.Startup()) < 0)
	{
		fprintf(stderr, "Network Startup failed: %d\n", ret);
		return -1;
	}
	if((ret = net.Bind()) < 0)
	{
		fprintf(stderr, "Network Bind failed: %d\n", ret);
		return -1;
	}

	while(true)
	{
		if((ret = net.Recv(recv_buf, msg_len)) < 0)
		{
			fprintf(stderr, "Network Recv failed: %d\n", ret);
			return -1;
		}

		if((ret = HandleMsg(net, chunk_db)) < 0)
		{
			fprintf(stderr, "HandleMsg failed: %d\n", ret);
			return -1;
		}
	}

	return 0;
}


