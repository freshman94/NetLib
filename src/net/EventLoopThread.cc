#include <net/EventLoopThread.h>
#include <net/EventLoop.h>

EventLoopThread::EventLoopThread(const string& name)
	: loop_(NULL),
	exiting_(false),
	thread_(std::bind(&EventLoopThread::threadFunc, this), name),
	mutex_(),
	cond_(mutex_)
{}

EventLoopThread::~EventLoopThread(){
	exiting_ = true;
	if (loop_ != NULL) {
		loop_->quit();
		thread_.join();
	}
}

EventLoop* EventLoopThread::startLoop(){
	assert(!thread_.started());
	thread_.start();

	EventLoop* loop = NULL;
	{
		MutexLockGuard lock(mutex_);
		//cond保证threadFunc真正地启动起来，本函数才返回
		while (loop_ == NULL)
			cond_.wait();
		loop = loop_;
	}
	return loop;
}

void EventLoopThread::threadFunc(){
	EventLoop loop;
	{
		MutexLockGuard lock(mutex_);
		loop_ = &loop;
		cond_.notify();
	}
	loop.loop();
	MutexLockGuard lock(mutex_);
	loop_ = NULL;
}

