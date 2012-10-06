//���������ñ������ݻ�ԭ���ݵ��ļ�
//������������������Ҫ��ԭ�ļ��Ŀ���Ϣ�ڿ���Ϣ�ļ��е�ƫ�ƣ����ļ����ݿ����

#include <cstdio>
#include <cstring>

const char *kFilesComposePath = "/root/DDRBS/bkinfo/files_compose"; //���ݿ���Ϣ�ļ�
const char *kFileBkPath = "/root/Desktop/bkfile"; //��ԭ���ı����ļ���·��
const char *kContainerPathPrefix = "/root/DDRBS/container/data"; //�����ļ�ǰ׺

char file_buf[1000000];
char chunk_buf[100000];

int get_chunk(unsigned id, unsigned offset, unsigned short len)
{
	char path[256];
	char tmp[8];

	sprintf(tmp, "%u", id);
	strcpy(path, kContainerPathPrefix);
	strcat(path, tmp);

	FILE *fp = fopen(path, "rb");
	if(fp == NULL)
		return -1;

	fseek(fp, offset, SEEK_SET);
	unsigned ret = fread(chunk_buf, 1, len, fp);
	if(ret != len)
		return -2;

	fclose(fp);

	return 1;
}

int get_backup_file(unsigned offset, unsigned count)
{
	FILE *fp = fopen(kFilesComposePath, "rb");
	if(fp == NULL)
		return -1;

	fseek(fp, offset, SEEK_SET);
	unsigned ret = fread(file_buf, 1, count*10, fp);
	if(ret != count*10)
		return -2;

	fclose(fp);

	fp = fopen(kFileBkPath, "wb");
	if(fp == NULL)
		return -3;

	for(unsigned i = 0; i < count; i++)
	{
		unsigned id = 0;
		unsigned offset = 0;
		unsigned short len = 0;

		memcpy(&id, file_buf+i*10, 4);
		memcpy(&offset, file_buf+i*10+4, 4);
		memcpy(&len, file_buf+i*10+8, 2);

		if(get_chunk(id, offset, len) < 0)
			return -4;
		fwrite(chunk_buf, 1, len, fp);
	}
	fclose(fp);

	return 1;
}

int main(int argc, char **argv)
{
	unsigned offset;
	unsigned count;

	if(argc != 3)
	{
		printf("usage\n");
		return -1;
	}

	sscanf(argv[1], "%u", &offset);
	sscanf(argv[2], "%u", &count);

	int ret = 0;
	if((ret = get_backup_file(offset, count)) < 0)
	{
		printf("failed: %d\n", ret);
		return -1;
	}

	return 0;
}
