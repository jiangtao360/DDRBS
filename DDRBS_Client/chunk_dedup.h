#ifndef DDRBS_CHUNK_DEDUP_H
#define DDRBS_CHUNK_DEDUP_H

#include <cstdio>
#include <cstring>
#include <vector>
#include <string>
#include "utility.h"
#include "network.h"

using std::vector;
using std::string;

class ChunkDedup
{
public:
	ChunkDedup(string chunk_file_path, string inc_file_path);
	~ChunkDedup();

	int DedupChunk(vector<ChunkInfo> &partitions, Network &net);

private:

private:
	string chunk_file_path_;
	//string ind_file_path_;
	string inc_file_path_;
};

#endif

