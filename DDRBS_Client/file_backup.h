#ifndef DDRBS_FILE_BACKUP_H
#define DDRBS_FILE_BACKUP_H

#include <string>
#include "utility.h"
#include "network.h"

using std::string;

class FileBackup
{
public:
	FileBackup(string inc_file_path);
	~FileBackup();
	int BackupIncFile(Network &net);


private:
	string inc_file_path_;
};

#endif

