#include "file_partition.h"
#include "sha1.h"

static Byte file_buf[kFileBufSize];

FilePartition::FilePartition(string file_path, Uint4 exp_chunk_size,
							 Uint4 min_chunk_size, Uint4 max_chunk_size) :
							 file_path_(file_path), exp_chunk_size_(exp_chunk_size), 
							 min_chunk_size_(min_chunk_size), max_chunk_size_(max_chunk_size), 
							 rabin_window_size_(kRabinWindowSize), rabin_mod_(kRabinMod)
{
	rabin_high_weight_ = 256;
	for(Uint4 i=0; i<rabin_window_size_-2; i++)
	{
		rabin_high_weight_ = (rabin_high_weight_ << 8) % rabin_mod_;
	}
	
	int value_bits = 0;
	unsigned int n = exp_chunk_size_-1;
	while(n)
	{
		n >>= 1;
		value_bits++;
	}

	chunk_mask_ = 1;
	for(int i=0; i<value_bits-1; i++)
		chunk_mask_ = (chunk_mask_<<1)+1;

	partition_magic_value_ = kPartitionMagicValue & chunk_mask_;
}

FilePartition::~FilePartition()
{

}

int FilePartition::PartitionFile(vector<ChunkInfo> &partitions)
{
	FILE *fp = fopen(file_path_.c_str(), "rb");
	if(fp == NULL)
		return -1;

	Uint4 ret = fread(file_buf, 1, kFileReadLen, fp);
	if(ret < min_chunk_size_)
	{
		ChunkInfo chunk;
		chunk.offset = 0;
		chunk.len = ret;
		sha1(file_buf, chunk.len, chunk.sha1);
	
		partitions.push_back(chunk);

		fclose(fp);

		return 1;
	}

	vector<Uint4> seed_points;
	seed_points.push_back(0);
	
	Uint4 last_rabin = RabinFirst(file_buf);
	Uint4 last_high_value = file_buf[0];
	SeekSeedPoint(seed_points, last_rabin, last_high_value, 
				  file_buf, 1, ret-rabin_window_size_+1, 0);

	Uint4 len = ret;
	Uint4 offset = ret-rabin_window_size_;
	while(true)
	{
		memcpy(file_buf, file_buf+len-rabin_window_size_, rabin_window_size_);
		
		ret = fread(file_buf+rabin_window_size_, 1, kFileReadLen-rabin_window_size_, fp);
		if(ret == 0)
			break;

		len = ret+rabin_window_size_;
		SeekSeedPoint(seed_points, last_rabin, last_high_value, 
				  file_buf, 1, len-rabin_window_size_+1, offset);

		offset += ret;

		if(ret < kFileReadLen-rabin_window_size_)
			break;
	}

	struct stat stat_buf;
	fstat(fileno(fp), &stat_buf);
	Uint4 file_size = stat_buf.st_size;
	
	fclose(fp);

	SeekChunk(seed_points, partitions, file_size);

	if(SetChunkFingerprint(partitions) < 0)
		return -2;

	return 1;
	
}

Uint4 FilePartition::RabinFirst(Byte *buf)
{
	Uint4 res = buf[0];
	
	for(Uint4 i=1; i<rabin_window_size_; i++)
	{
		res = ((res << 8) + buf[i]) % rabin_mod_;
	}

	return res;
}

void FilePartition::SeekSeedPoint(vector<Uint4> &seed_points, Uint4 &last_rabin, Uint4 &last_high_value,
						const Byte *buf, Uint4 start, Uint4 end, Uint4 offset)
{
	for(Uint4 i = start; i < end; i++)
	{
		Uint4 current_rabin = ((((rabin_mod_ + last_rabin - 
		last_high_value * rabin_high_weight_ % rabin_mod_) % rabin_mod_) << 8) + 
								buf[i + rabin_window_size_ - 1]) % rabin_mod_;

		if((current_rabin & chunk_mask_) == partition_magic_value_)
			seed_points.push_back(i+offset);

		last_rabin = current_rabin;
		last_high_value = buf[i];
	}
}

void FilePartition::SeekChunk(vector<Uint4> &seed_points, vector<ChunkInfo> &partitions, Uint4 file_size)
{
	Uint4 points_count = seed_points.size();
	Uint4 last_pos = 0;

	for(Uint4 i=1; i<points_count; i++)
	{
		if(seed_points[i] - last_pos < min_chunk_size_)
		{
			continue;
		}
		else if(seed_points[i] - last_pos > max_chunk_size_)
		{
			ChunkInfo chunk;
			chunk.offset = last_pos;
			chunk.len = max_chunk_size_;
			partitions.push_back(chunk);

			last_pos += max_chunk_size_;
			i--;
		}
		else
		{
			ChunkInfo chunk;
			chunk.offset = last_pos;
			chunk.len = seed_points[i] - last_pos;
			partitions.push_back(chunk);

			last_pos = seed_points[i];
		}
	}

	while(true)
	{
		ChunkInfo chunk;
		chunk.offset = last_pos;
		chunk.len = (file_size-last_pos) <= max_chunk_size_ ? (file_size-last_pos) : max_chunk_size_;
		partitions.push_back(chunk);

		if(chunk.len == file_size-last_pos)
			break;
		last_pos += max_chunk_size_;
	}
	
}

int FilePartition::SetChunkFingerprint(vector<ChunkInfo> &partitions)
{
	Uint4 chunk_count = partitions.size();

	FILE *fp = fopen(file_path_.c_str(), "rb");
	if(fp == NULL)
		return -1;

	for(Uint4 i=0; i<chunk_count; i++)
	{
		int ret = fread(file_buf, 1, partitions[i].len, fp);
		if(ret < partitions[i].len)
			return -2;

		sha1(file_buf, ret, partitions[i].sha1);
	}

	fclose(fp);

	return 1;
}





