#include "FileUtil.h"

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

//fopen:type为a时，其标志位为 O_WRONLY|O_CREAT|O_APPEND

AppendFile::AppendFile(StringArg filename)
	: fp_(fopen(filename.c_str(),"ae")),	//'e'表示 O_CLOEXEC
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
	//为了快速，使用unlocked(无锁)的fwrite函数。平时我们使用的C语言IO函数，都是线程安全的，
	//为了做到线程安全，会在函数的内部加锁。这会拖慢速度。而对于这个类。可以保证，从
	//始到终只有一个线程能访问，所以无需进行加锁操作。
	return fwrite_unlocked(logline, 1, len, fp_);
}