#pragma once

#include <base/Condition.h>
#include <base/Mutex.h>

//CountDownLatch��һ��ͬ�������࣬������һ�������߳�һֱ�ȴ���ֱ�������̵߳Ĳ���ִ�������ִ�С�
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
