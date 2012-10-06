#include "chunk_db.h"


ChunkDB::ChunkDB(string db_path) : db_path_(db_path)
{
}

ChunkDB::~ChunkDB()
{
}

int ChunkDB::OpenDB()
{
	leveldb::Options options;
  	options.create_if_missing = true;

  	
	leveldb::Status status = leveldb::DB::Open(options, db_path_, &chunk_db_);

	if(status.ok())
	{
		return 1;
	}
	else
	{
		return -1;
	}
}

int ChunkDB::CloseDB()
{
	delete chunk_db_;

	return 1;
}

int ChunkDB::Get(string &key, string *value)
{
	leveldb::Status status = chunk_db_->Get(leveldb::ReadOptions(), key, value);

	if(status.ok())
		return 1;
	else
		return -1;
}

int ChunkDB::Put(string &key, string &value)
{	
	leveldb::Status status = chunk_db_->Put(leveldb::WriteOptions(), key, value);

	if(status.ok())
		return 1;
	else
		return -1;
}



