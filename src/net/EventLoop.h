#pragma once

#include <base/Mutex.h>
#include <base/CurrentThread.h>
#include <net/TimerQueue.h>

#include <functional>
#include <vector>
#include <memory>
#include <boost/any.hpp>

class Channel;
class Epoll;

//反应器（Reactor）
class EventLoop : noncopyable{
public:
	typedef std::function<void()> Functor;

	EventLoop();
	~EventLoop();
	void loop();
	void quit();

	void runInLoop(Functor cb);
	void queueInLoop(Functor cb);

	size_t queueSize() const;

	void runAt(Timestamp time, TimerCallback cb);
	void runAfter(double delay, TimerCallback cb);
	void runEvery(double interval, TimerCallback cb);

	void wakeup();
	void updateChannel(Channel* channel);
	void removeChannel(Channel* channel);
	bool hasChannel(Channel* channel);

	//保证是在EventLoop所在的线程中
	void assertInLoopThread(){
		if (!isInLoopThread())
			abortNotInLoopThread();
	}
	bool isInLoopThread() const { return threadId_ == CurrentThread::tid(); }
	bool eventHandling() const { return eventHandling_; }

	void setContext(const boost::any& context){ context_ = context;}
	const boost::any& getContext() const{ return context_;}
	boost::any* getMutableContext(){ return &context_;}

	static EventLoop* getEventLoopOfCurrentThread();

private:
	void abortNotInLoopThread();
	void handleRead();
	void ExecPendingFuncs();

	typedef std::vector<Channel*> ChannelList;

	bool looping_; 
	bool quit_;
	bool eventHandling_;
	bool callingPendingFunctors_;
	const pid_t threadId_;
	std::unique_ptr<Epoll> epoll_;
	std::unique_ptr<TimerQueue> timerQueue_;
	int wakeupFd_;	//eventfd,实现异步唤醒EventLoop功能
	std::unique_ptr<Channel> wakeupChannel_;
	boost::any context_;

	ChannelList activeChannels_;
	Channel* currentActiveChannel_;

	mutable MutexLock mutex_;
	//用于存储
	std::vector<Functor> pendingFunctors_;
};
