#pragma once

#include <net/Timestamp.h>

typedef std::function<void()> TimerCallback;

class Timer : noncopyable {
public:
	Timer(TimerCallback cb, Timestamp when, double interval)
		: callback_(std::move(cb)),
		expiration_(when),
		interval_(interval),
		repeat_(interval > 0.0)
	{ }

	void run() const { callback_(); }

	Timestamp expiration() const { return expiration_; }
	bool repeat() const { return repeat_; }

	void restart(Timestamp now) {
		if (repeat_)
			expiration_ = addTime(now, interval_);
		else
			expiration_ = Timestamp::invalid();
	}

private:
	const TimerCallback callback_;
	Timestamp expiration_;
	const double interval_;
	const bool repeat_;
};
