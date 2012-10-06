//�����������޸�һ��Ŀ¼�µĶ���ļ����޸�һ���ļ�ʱ���ڸ��ļ��ض���λ�ø���һ���ֽ�
//�����������������ֱ�Ϊ:Ŀ¼��·������Ŀ¼��֮ǰ�Ѿ��޸ĵ��ļ������������Ҫ�޸ĵ��ļ�����

#include <cstdio>
#include <string>
#include <vector>
#include <sys/stat.h>
#include <ftw.h>

using std::string;
using std::vector;

vector<string> files;
unsigned char magic_byte = 69; //�޸�һ���ļ�ʱ�����滻���ֽ�ֵ

int get_file_info(const char *fpath, const struct stat *finfo, int typeflag, struct FTW *ftwbuf)
{
	if(typeflag == FTW_F)
	{
		files.push_back(fpath);
	}

	return 0;
}

int update_file(const char *path, size_t index)
{
	FILE *fp = fopen(path, "r+");
	if(fp == NULL)
		return -1;

	struct stat stat_buf;
	fstat(fileno(fp), &stat_buf);
	size_t file_size = stat_buf.st_size;

	index %= file_size;

	fseek(fp, index, SEEK_SET);
	if(fwrite(&magic_byte, 1, 1, fp) != 1)
		return -2;

	fclose(fp);

	return 1;
}

int change_files(char *path, unsigned before, unsigned now)
{
	int ret = 0;

	if((ret = nftw(path, get_file_info, 128, 0)) != 0)
		return -1;

	
	//for(size_t i = 0; i < files.size(); i++)
	//	printf("%s\n", files[i].c_str());

	size_t end = now < (files.size() - before) ? (before + now) : files.size();
	for(size_t i = before; i < end; i++)
	{
		if(update_file(files[i].c_str(), i) < 0)
			return -2;
	}

	return 1;
}

int main(int argc, char **argv)
{
	unsigned before = 0;
	unsigned now = 0;

	if(argc != 4)
	{
		printf("usage path before now\n");
		return -1;
	}

	sscanf(argv[2], "%u", &before);
	sscanf(argv[3], "%u", &now);

	if(change_files(argv[1], before, now) < 0)
	{
		printf("change_files failed\n");
		return -1;
	}

	return 0;
}
