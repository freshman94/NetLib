#pragma once

#include <base/CountDownLatch.h>
#include <base/Mutex.h>
#include <base/Thread.h>
#include <log/LogStream.h>

#include <vector>

class AsyncLogging : noncopyable{
public:

	AsyncLogging(const string& basename,
		off_t rollSize,
		int flushInterval = 3);

	~AsyncLogging(){
		if (running_)
			stop();
	}

	void append(const char* logline, int len);

	//CountDownLatch是为了确保thread的func真正地启动起来，这时start函数才能返回。
	void start(){
		running_ = true;
		thread_.start();
		latch_.wait();
	}

	void stop() {
		running_ = false;
		cond_.notify();
		thread_.join();
	}

private:

	void threadFunc();

	typedef FixedBuffer<kLargeBuffer> Buffer;
	typedef std::vector<std::unique_ptr<Buffer>> BufferVector;
	typedef std::unique_ptr<Buffer> BufferPtr;

	const int flushInterval_;
	bool running_;
	const string basename_;
	const off_t rollSize_;
	Thread thread_;
	CountDownLatch latch_;
	MutexLock mutex_;
	Condition cond_;
	BufferPtr currentBuffer_;
	BufferPtr nextBuffer_;
	BufferVector buffers_;
};
