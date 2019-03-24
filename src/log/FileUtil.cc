#include "FileUtil.h"

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

//fopen:typeΪaʱ�����־λΪ O_WRONLY|O_CREAT|O_APPEND

AppendFile::AppendFile(StringArg filename)
	: fp_(fopen(filename.c_str(),"ae")),	//'e'��ʾ O_CLOEXEC
	writtenBytes_(0)
{
	assert(fp_);
	setbuffer(fp_, buffer_, sizeof buffer_);
}

AppendFile::~AppendFile() {
	fclose(fp_);
}

void AppendFile::append(const char* logline, const size_t len) {
	size_t n = this->write(logline, len);
	size_t remain = len - n;
	while (remain > 0) {
		size_t x = this->write(logline, len);
		if (x == 0) {
			int err = ferror(fp_);
			if (err)
				fprintf(stderr, "AppendFile::append() failed %s\n", strerror(err));
			break;
		}
		n += x;
		remain = len - n;
	}
	writtenBytes_ += len;
}

void AppendFile::flush() {
	fflush(fp_);
}

size_t AppendFile::write(const char* logline, size_t len)
{
	//Ϊ�˿��٣�ʹ��unlocked(����)��fwrite������ƽʱ����ʹ�õ�C����IO�����������̰߳�ȫ�ģ�
	//Ϊ�������̰߳�ȫ�����ں������ڲ���������������ٶȡ�����������ࡣ���Ա�֤����
	//ʼ����ֻ��һ���߳��ܷ��ʣ�����������м���������
	return fwrite_unlocked(logline, 1, len, fp_);
}