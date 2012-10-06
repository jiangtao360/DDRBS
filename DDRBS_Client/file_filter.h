#ifndef DDRBS_FILE_FILTER_H
#define DDRBS_FILE_FILTER_H

//#define _XOPEN_SOURCE 500
#include <cstdio>
#include <cstring>
#include <vector>
#include <string>
#include <ftw.h>

using std::vector;
using std::string;
using std::sort;

struct FileInfo
{
	FileInfo(const char *p, time_t t) : path(p), mtime(t) {}
		
	string path;
	unsigned int mtime;
};

class FileFilter
{
public:
	FileFilter(string backup_files_path, string backup_record_path);
	~FileFilter();

	int FilterFiles(vector<string> &backup_files);
	

private:
	int GetCurrentFiles();
	static int GetFileInfo(const char *fpath, const struct stat *finfo, 
					   int typeflag, struct FTW *ftwbuf);
	static bool FileCmp(FileInfo a, FileInfo b);
	int GetLastFiles();
	int GenerateRecordFile();
	int GetBackupFiles(vector<string> &backup_files);
	
private:
	static vector<FileInfo> current_files_;
	static vector<FileInfo> last_files_;
	string backup_files_path_;
	string backup_record_path_;
};

#endif

