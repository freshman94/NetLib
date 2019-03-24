#pragma once

#include <base/Mutex.h>
#include <base/Types.h>

#include <memory>

class AppendFile;	//前向声明

class LogFile : noncopyable {
public:
	//每隔3秒或者每调用checkEveryN次append，就flush一次，将缓冲空间中的日志写入文件中
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