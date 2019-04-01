#pragma once
#include <set>
#include <vector>
#include <memory>
#include <queue>

#include <base/Mutex.h>
#include <net/Timestamp.h>
#include <net/Channel.h>
#include <net/Timer.h>

class EventLoop;

struct cmp
{
	bool operator()(const std::shared_ptr<Timer>& lhs, const std::shared_ptr<Timer>& rhs)
	{
		return lhs->expiration() > rhs->expiration();
	}
};

class TimerQueue : noncopyable {
public:
	explicit TimerQueue(EventLoop* loop);
	~TimerQueue();

	void addTimer(TimerCallback cb,
		Timestamp when, double interval);

private:
	typedef std::shared_ptr<Timer> TimerPtr;
	void addTimerInLoop(TimerPtr timer);
	void handleExpired();	//处理超时事件
	void reset(const std::vector<TimerPtr>& expired, Timestamp now);

	std::priority_queue<TimerPtr, std::vector<TimerPtr>, cmp> TimerPq_;
	std::vector<TimerPtr> expired_;
	EventLoop* loop_;
	const int timerfd_;
	Channel timerfdChannel_;	
};
