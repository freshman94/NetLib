#pragma once

#include <base/Condition.h>
#include <base/Mutex.h>
#include <base/Thread.h>

class EventLoop;
//����һ���̣߳�����������EventLoop
class EventLoopThread : noncopyable{
public:
	EventLoopThread(const string& name = string());
	~EventLoopThread();
	EventLoop* startLoop();

private:
	void threadFunc();

	EventLoop* loop_;
	bool exiting_;
	Thread thread_;
	MutexLock mutex_;
	Condition cond_;
};

