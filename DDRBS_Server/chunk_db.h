#ifndef DDRBS_CHUNK_DB_H
#define DDRBS_CHUNK_DB_H

#include <string>
#include "leveldb/db.h"

using std::string;

class ChunkDB
{
public:
	ChunkDB(string db_path);
	~ChunkDB();

	int OpenDB();
	int CloseDB();
	int Get(string &key, string *value);
	int Put(string &key, string &value);

private:
	string db_path_;
	leveldb::DB *chunk_db_;
};

#endif

