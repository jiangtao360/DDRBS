#ifndef DDRBS_UTILITY_H
#define DDRBS_UTILITY_H

//#define DEBUG_

typedef unsigned int ChunkOffset;
typedef unsigned char Byte;
typedef unsigned int Uint4;

const Uint4 kMaxPathLen = 256;
const Uint4 kFileBufSize = 1048576;
const Uint4 kNetBufSize = 1048576;


struct ChunkInfo
{
	unsigned short len;
	bool is_duplication;
	Uint4 container_id;
	Uint4 container_offset;
};

enum MsgType
{
	BACKUP_START_REQ, // 0
	BACKUP_START_REP, // 1


	CHUNK_START_REQ, // 2
	CHUNK_START_REP, // 3
	CHUNK_INFO_REQ, // 4
	CHUNK_INFO_REP, // 5
	CHUNK_END_REQ,  // 6
	CHUNK_END_REP, //7
/*
	IND_START_REQ,
	IND_START_REP,
	IND_DATA_REQ,
	IND_DATA_REP, //reserve
	IND_END_REQ, 
	IND_END_REP,
	*/
	INC_START_REQ, //8
	INC_START_REP, //9
	INC_DATA_REQ, // 10
	INC_DATA_REP, // 11
	INC_END_REQ, // 12
	INC_END_REP, // 13


	BACKUP_END_REQ, //14
	BACKUP_END_REP  //15
};

#endif


