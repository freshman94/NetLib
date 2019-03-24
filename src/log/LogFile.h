#pragma once

#include <base/Mutex.h>
#include <base/Types.h>

#include <memory>

class AppendFile;	//ǰ������

class LogFile : noncopyable {
public:
	//ÿ��3�����ÿ����checkEveryN��append����flushһ�Σ�������ռ��е���־д���ļ���
	LogFile(const string& basename,
			off_t rollSize,
			bool threadSafe = true,
			int flushInterval = 3,
			int checkEveryN = 1024);
	~LogFile();

	void append(const char* logline, int len);
	void flush();
	bool rollFile();

private:
	void append_unlocked(const char* logline, int len);
	static string getLogFileName(const string& basename, time_t* now);

	const string basename_;
	const off_t rollSize_;
	const int flushInterval_;
	const int checkEveryN_;

	int count_;

	std::unique_ptr<MutexLock> mutex_;
	time_t lastRoll_;
	time_t lastFlush_;
	std::unique_ptr<AppendFile> file_;
};