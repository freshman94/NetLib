#include <log/LogFile.h>
#include <log/FileUtil.h>

#include <assert.h>
#include <stdio.h>
#include <time.h>

LogFile::LogFile(const string& basename,
	off_t rollSize,
	bool threadSafe,
	int flushInterval,
	int checkEveryN)
	: basename_(basename),
	rollSize_(rollSize),
	flushInterval_(flushInterval),
	checkEveryN_(checkEveryN),
	count_(0),
	mutex_(threadSafe ? new MutexLock : NULL),
	lastRoll_(0),
	lastFlush_(0) 
{
	assert(basename.find('/') == string::npos);
	rollFile();
}

LogFile::~LogFile() {}

void LogFile::append(const char* logline, int len) {
	if (mutex_) {
		MutexLockGuard lock(*mutex_);
		append_unlocked(logline, len);
	}
	else
		append_unlocked(logline, len);
}

void LogFile::flush(){
	if (mutex_)
	{
		MutexLockGuard lock(*mutex_);
		file_->flush();
	}
	else
		file_->flush();
}

void LogFile::append_unlocked(const char* logline, int len) {
	file_->append(logline, len);

	//若日志文件大小超过rollSize_就滚动日志
	//Flush间隔超过flushInterval_，就flush缓冲区
	if (file_->writtenBytes() > rollSize_)
		rollFile();
	else {
		++count_;
		if (count_ >= checkEveryN_) {
			count_ = 0;
			time_t now = time(NULL);
			if (now - lastFlush_ > flushInterval_) {
				lastFlush_ = now;
				file_->flush();
			}
		}
	}
}

bool LogFile::rollFile(){
	time_t now = 0;
	string filename = getLogFileName(basename_, &now);

	if (now > lastRoll_){
		lastRoll_ = now;
		lastFlush_ = now;
		file_.reset(new AppendFile(filename));
		return true;
	}
	return false;
}

//日志文件名的格式为：logfile.20120603-144022.log

string LogFile::getLogFileName(const string& basename, time_t* now){
	string filename;
	filename.reserve(basename.size() + 64);
	filename = basename;

	char timebuf[32];
	struct tm tm;
	*now = time(NULL);
	localtime_r(now, &tm);
	strftime(timebuf, sizeof timebuf, ".%Y%m%d-%H%M%S", &tm);
	filename += timebuf;

	filename += ".log";

	return filename;
}