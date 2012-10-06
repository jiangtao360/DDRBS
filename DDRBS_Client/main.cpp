#include <cstdio>
#include <cstring>
#include <zmq.h>
#include "utility.h"
#include "file_filter.h"
#include "file_partition.h"
#include "network.h"
#include "chunk_dedup.h"
#include "file_backup.h"

const char *kIncFilePath = "inc_tmp";

static int NotifyBackupStart(Network &net, char *backup_path)
{
	char buf[128];

	buf[0] = BACKUP_START_REQ;
	Uint4 path_len = strlen(backup_path);
	memcpy(buf+1, backup_path, path_len+1);

	if(net.Send(buf, path_len+2) < 0)
	{
		return -1;
	}

	Uint4 recv_len = 0;
	if(net.Recv(buf, recv_len) < 0)
	{
		return -2;
	}

	if(buf[0] != BACKUP_START_REP || buf[1] != 1)
	{
		return -3;
	}

	return 1;
}

static int NotifyBackupEnd(Network & net)
{
	char buf[128];

	buf[0] = BACKUP_END_REQ;
	if(net.Send(buf, 1) < 0)
		return -1;

	Uint4 recv_len = 0;
	if(net.Recv(buf, recv_len) < 0 || buf[0] != BACKUP_END_REP)
	{
		return -2;
	}

	return 1;
}
/*
static void Sha1String(Byte *sha1, char *str)
{
	char tmp[4];

	str[0] = 0;
	for(int i = 0; i < 20; i++)
	{
		sprintf(tmp, "%02X", sha1[i]);
		strcat(str, tmp);
	}
}*/

int main(int argc, char **argv)
{
	if(argc != 2)
	{
		fprintf(stderr, "usage: ddrbs_cli path!\n");
		return -1;
	}
	int ret = 0;

	Network net("192.168.13.234", "969", ZMQ_REQ);
	if((ret = net.Startup()) < 0)
	{
		fprintf(stderr, "Network Startup failed: %d\n", ret);
		return -1;
	}
	if((ret = net.Connect()) < 0)
	{
		fprintf(stderr, "Network Connect failed: %d\n", ret);
		return -1;
	}

	if((ret = NotifyBackupStart(net, argv[1])) < 0)
	{
		fprintf(stderr, "NofifyBackup failed: %d\n", ret);
		return -1;
	}
	
	vector<string> backup_files;
	FileFilter filter(argv[1], "record");
	if((ret = filter.FilterFiles(backup_files)) < 0 )
	{
		fprintf(stderr, "FilterFiles failed: %d\n", ret);
		return -1;
	}
	
	int files_count = backup_files.size();
	for(int i=0; i<files_count; i++)
	{
		#ifdef DEBUG_
		printf("file: %s\n", backup_files[i].c_str());
		#endif

		FilePartition file_partition(backup_files[i], 8192, 1024, 16384);
		vector<ChunkInfo> partitions;
		ret = file_partition.PartitionFile(partitions);
		if(ret < 0)
		{
			fprintf(stderr, "PartitionFile failed: %d\n", ret);
			return -1;
		}

		#ifdef DEBUG_
		for(size_t i=0; i<partitions.size(); i++)
		{
			printf("%d ", partitions[i].len);
		}
		printf("\n");
		#endif


		ChunkDedup dedup(backup_files[i], kIncFilePath);
		ret = dedup.DedupChunk(partitions, net);
		if(ret < 0)
		{
			fprintf(stderr, "DedupChunk failed: %d\n", ret);
			return -1;
		}

		FileBackup fb(kIncFilePath);
		ret = fb.BackupIncFile(net);
		if(ret < 0)
		{
			fprintf(stderr, "BackupIncFile failed: %d\n", ret);
			return -1;
		}
		
	}

	if((ret = NotifyBackupEnd(net)) < 0)
	{
		fprintf(stderr, "NotifyBackupEnd failed: %d\n", ret);
		return -1;
	}

	net.PrintFlow();
	net.Close();

	return 0;
}
