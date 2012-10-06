#include "file_filter.h"

vector<FileInfo> FileFilter::current_files_;
vector<FileInfo> FileFilter::last_files_;

FileFilter::FileFilter(string backup_files_path, string backup_record_path) : 
backup_files_path_(backup_files_path), backup_record_path_(backup_record_path)
{
	current_files_.clear();
	last_files_.clear();
}

FileFilter::~FileFilter()
{
}

int FileFilter::FilterFiles(vector<string> &backup_files)
{
	int ret = 0;
	
	if((ret = GetCurrentFiles()) < 0)
	{
		return -1;
	}
	
	if((ret = GetLastFiles())< -1)
	{
		return -2;
	}

	if((ret = GenerateRecordFile()) < 0)
	{
		return -3;
	}

	if((ret = GetBackupFiles(backup_files)) < 0)
	{
		return -4;
	}

	return 1;
}

int FileFilter::GetCurrentFiles()
{
	int ret = 0;

	if((ret = nftw(backup_files_path_.c_str(), GetFileInfo, 128, 0)) != 0)
	{
		return ret;
	}
	else
	{
		sort(current_files_.begin(), current_files_.end(), FileCmp);
		return 1;
	}
}

int FileFilter::GetFileInfo(const char *fpath, const struct stat *finfo, 
					   int typeflag, struct FTW *ftwbuf)
{
	if(typeflag == FTW_F)
	{
		current_files_.push_back(FileInfo(fpath, (unsigned int)finfo->st_mtime));
	}

	return 0;
}

bool FileFilter::FileCmp(FileInfo a, FileInfo b)
{
	return a.path < b.path;
}

int FileFilter::GetLastFiles()
{
	FILE *fp = fopen(backup_record_path_.c_str(), "r");
	if(fp == NULL)
		return -1;

	char buf[256];
	unsigned int tt;
	while(fscanf(fp, "%s %u", buf, &tt) == 2)
	{
		last_files_.push_back(FileInfo(buf, tt));
	}

	fclose(fp);

	return 1;
}

int FileFilter::GenerateRecordFile()
{
	FILE *fp = fopen(backup_record_path_.c_str(), "w");
	if(fp == NULL)
		return -1;

	int len = current_files_.size();
	for(int i=0; i<len; i++)
	{
		if(fprintf(fp, "%s %u\n", current_files_[i].path.c_str(), current_files_[i].mtime) < 0)
			return -2;
	}

	fclose(fp);

	return 1;
}

int FileFilter::GetBackupFiles(vector<string> &backup_files)
{
	int current_len = current_files_.size();
	int last_len = last_files_.size();

	int i = 0;
	int j = 0;
	while(i<current_len && j<last_len)
	{
		while(current_files_[i].path < last_files_[i].path)
		{
			backup_files.push_back(current_files_[i++].path);
		}

		if((current_files_[i].path == last_files_[j].path)) 
		{
			if(current_files_[i].mtime > last_files_[j].mtime)
			{
				backup_files.push_back(current_files_[i].path);
			}
			i++;
		}

		j++;
	}

	for(; i<current_len; i++)
		backup_files.push_back(current_files_[i].path);

	return 1;
}




