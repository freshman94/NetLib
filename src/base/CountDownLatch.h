#pragma once

#include <base/Condition.h>
#include <base/Mutex.h>

//CountDownLatch是一个同步工具类，它允许一个或多个线程一直等待，直到其他线程的操作执行完后再执行。
class CountDownLatch : noncopyable{
public:
	explicit CountDownLatch(int count);
	void wait();
	void countDown();
	int getCount() const;

private:
	mutable MutexLock mutex_;
	Condition condition_;
	int count_;
};
