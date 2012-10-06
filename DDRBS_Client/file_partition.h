#ifndef DDRBS_FILE_PARTITION_H
#define DDRBS_FILE_PARTITION_H

#include <cstdio>
#include <cstring>
#include <vector>
#include <string>
#include <sys/stat.h>
#include "utility.h"

using std::vector;
using std::string;

class FilePartition
{
public:
	FilePartition(string file_path, Uint4 exp_chunk_size, 
				   Uint4 min_chunk_size, Uint4 max_chunk_size);
	~FilePartition();
	
	int PartitionFile(vector<ChunkInfo> &partitions);

private:
	Uint4 RabinFirst(Byte *buf);
	void SeekSeedPoint(vector<Uint4> &seed_points, Uint4 &last_rabin, Uint4 &last_high_value,
							const Byte *buf, Uint4 start, Uint4 end, Uint4 offset);
	void SeekChunk(vector<Uint4> &seed_points, vector<ChunkInfo> &partitions, Uint4 file_size);
	int SetChunkFingerprint(vector<ChunkInfo> &partitions);
	
private:
	string file_path_;
	Uint4 exp_chunk_size_;
	Uint4 min_chunk_size_;
	Uint4 max_chunk_size_;
	Uint4 rabin_window_size_;
	Uint4 rabin_mod_;
	Uint4 rabin_high_weight_;  //当计算rabin指纹时，最高位的数字乘的权值
	Uint4 chunk_mask_;
	Uint4 partition_magic_value_;
};

#endif 

