#ifndef DDRBS_UTILITY_H
#define DDRBS_UTILITY_H

//#define DEBUG_


typedef unsigned int ChunkOffset;
typedef unsigned char Byte;
typedef unsigned int Uint4;

const Uint4 kMaxPathLen = 256;
const Uint4 kFileBufSize = 1048576;
const Uint4 kFileReadLen = 8192;
const Uint4 kRabinWindowSize = 8; // 4bytes~12bytes
const Uint4 kRabinMod = 16777199;
const Uint4 kPartitionMagicValue = 0; // Ò»°ãÖµÎª0
const Uint4 kNetBufSize = 1048576;

struct ChunkInfo
{
	ChunkOffset offset;
	unsigned short len;
	Byte sha1[20];
};

enum MsgType
{
	BACKUP_START_REQ,
	BACKUP_START_REP,


	CHUNK_START_REQ,
	CHUNK_START_REP,
	CHUNK_INFO_REQ,
	CHUNK_INFO_REP, //reserve
	CHUNK_END_REQ,
	CHUNK_END_REP,
/*
	IND_START_REQ,
	IND_START_REP,
	IND_DATA_REQ,
	IND_DATA_REP, //reserve
	IND_END_REQ, 
	IND_END_REP,
	*/
	INC_START_REQ,
	INC_START_REP,
	INC_DATA_REQ,
	INC_DATA_REP, //reserve
	INC_END_REQ,
	INC_END_REP,


	BACKUP_END_REQ,
	BACKUP_END_REP
};

#endif

