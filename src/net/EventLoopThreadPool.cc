#include <net/EventLoopThreadPool.h>
#include <net/EventLoop.h>
#include <net/EventLoopThread.h>
#include <log/Logger.h>

#include <stdio.h>

EventLoopThreadPool::EventLoopThreadPool(EventLoop* baseLoop, const string& nameArg, int numThreads)
	: baseLoop_(baseLoop),
	name_(nameArg),
	started_(false),
	numThreads_(numThreads),
	next_(0)
{
	if (numThreads_ < 0)
		LOG_FATAL << "EventLoopThreadPool::numThreads_ < 0";
}

EventLoopThreadPool::~EventLoopThreadPool()
{}

void EventLoopThreadPool::start(){
	assert(!started_);
	baseLoop_->assertInLoopThread();
	started_ = true;

	for (int i = 0; i < numThreads_; ++i){
		char buf[name_.size() + 32];
		snprintf(buf, sizeof buf, "%s%d", name_.c_str(), i);
		EventLoopThread* t = new EventLoopThread(buf);
		threads_.push_back(std::unique_ptr<EventLoopThread>(t));
		loops_.push_back(t->startLoop());
	}
}

EventLoop* EventLoopThreadPool::getNextLoop(){
	baseLoop_->assertInLoopThread();
	assert(started_);
	EventLoop* loop = baseLoop_;

	//Ñ­»·
	if (!loops_.empty()){
		loop = loops_[next_];
		next_ = (next_ + 1) % numThreads_;
	}
	return loop;
}
