#pragma once
#include <base/noncopyable.h>
#include <base/Types.h>

#include <memory>
#include <vector>

class EventLoop;
class EventLoopThread;

class EventLoopThreadPool : noncopyable{
public:
	EventLoopThreadPool(EventLoop* baseLoop, const string& nameArg, int numThreads);
	~EventLoopThreadPool();
	void start();
	EventLoop* getNextLoop();

private:
	EventLoop * baseLoop_;
	string name_;
	bool started_;
	int numThreads_;
	int next_;
	std::vector<std::unique_ptr<EventLoopThread>> threads_;
	std::vector<EventLoop*> loops_;
};

