#ifndef __STDC_LIMIT_MACROS
#define __STDC_LIMIT_MACROS
#endif

#include <net/TimerQueue.h>
#include <log/Logger.h>
#include <net/EventLoop.h>
#include <net/Timer.h>

#include <sys/timerfd.h>
#include <unistd.h>

int createTimerfd() {
	int timerfd = timerfd_create(CLOCK_MONOTONIC,	//¾ø¶ÔÊ±¼ä
						TFD_NONBLOCK | TFD_CLOEXEC);
	if (timerfd < 0)
		LOG_FATAL << "Failed in timerfd_create";
	return timerfd;
}

struct timespec howMuchTimeFromNow(Timestamp when){
	int64_t microseconds = when.microSecsSinceEpoch()
		- Timestamp::now().microSecsSinceEpoch();
	if (microseconds < 100)
		microseconds = 100;

	struct timespec ts;
	ts.tv_sec = static_cast<time_t>(
		microseconds / Timestamp::MicroSecsPerSec);
	ts.tv_nsec = static_cast<long>(
		(microseconds % Timestamp::MicroSecsPerSec) * 1000);
	return ts;
}

void readTimerfd(int timerfd, Timestamp now){
	uint64_t howmany;
	ssize_t n = read(timerfd, &howmany, sizeof howmany);
	LOG_TRACE << "TimerQueue::handleExpired() " << howmany << " at " << now.toString();
	if (n != sizeof howmany)
		LOG_ERROR << "TimerQueue::handleExpired() reads " << n << " bytes instead of 8";
}

void resetTimerfd(int timerfd, Timestamp expiration){
	struct itimerspec newValue;
	struct itimerspec oldValue;
	memZero(&newValue, sizeof newValue);
	memZero(&oldValue, sizeof oldValue);
	newValue.it_value = howMuchTimeFromNow(expiration);
	int ret = timerfd_settime(timerfd, 0, &newValue, &oldValue);
	if (ret)
		LOG_ERROR << "timerfd_settime()";
}



TimerQueue::TimerQueue(EventLoop* loop)
	: loop_(loop),
	timerfd_(createTimerfd()),
	timerfdChannel_(loop, timerfd_)
{
	timerfdChannel_.setReadCallback(std::bind(&TimerQueue::handleExpired, this));
	timerfdChannel_.enableReading();
}

TimerQueue::~TimerQueue()
{
	timerfdChannel_.disableAll();
	timerfdChannel_.remove();
	close(timerfd_);
}

void TimerQueue::addTimer(TimerCallback cb, Timestamp when, double interval)
{
	TimerPtr timer = std::make_shared<Timer>(std::move(cb), when, interval);
	loop_->runInLoop(std::bind(&TimerQueue::addTimerInLoop, this, timer));
}

void TimerQueue::addTimerInLoop(TimerPtr timer)
{
	loop_->assertInLoopThread();
	if (TimerPq_.empty() || timer->expiration() < TimerPq_.top()->expiration())
		resetTimerfd(timerfd_, timer->expiration());
	TimerPq_.push(timer);
}

void TimerQueue::handleExpired()
{
	loop_->assertInLoopThread();
	Timestamp now(Timestamp::now());
	readTimerfd(timerfd_, now);

	expired_.clear();
	while (!TimerPq_.empty() && TimerPq_.top()->expiration() < now) {
		TimerPq_.top()->run();
		expired_.push_back(TimerPq_.top());
		TimerPq_.pop();
	}

	reset(expired_, now);
}


void TimerQueue::reset(const std::vector<TimerPtr>& expired, Timestamp now)
{
	Timestamp nextExpire;
	for (const TimerPtr& it : expired){
		if (it->repeat()){
			it->restart(now);
			TimerPq_.push(it);
		}
	}

	if (!TimerPq_.empty())
		nextExpire = TimerPq_.top()->expiration();
	if (nextExpire.valid())
		resetTimerfd(timerfd_, nextExpire);
}

