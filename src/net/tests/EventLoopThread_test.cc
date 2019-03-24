#include <net/EventLoopThread.h>
#include <net/EventLoop.h>
#include <base/Thread.h>
#include <base/CountDownLatch.h>

#include <stdio.h>
#include <unistd.h>

void sleepUsec(int64_t usec) {
	struct timespec ts = { 0, 0 };
	ts.tv_sec = static_cast<time_t>(usec / (1000 * 1000));
	ts.tv_nsec = static_cast<long>(usec % (1000 * 1000) * 1000);
	nanosleep(&ts, NULL);
}

void print(EventLoop* p = NULL) {
	printf("print: pid = %d, tid = %d, loop = %p\n",
		getpid(), CurrentThread::tid(), p);
}

void quit(EventLoop* p) {
	print(p);
	p->quit();
}

int main() {
	print();

	{
		EventLoopThread thr1;  // never start
	}

	{
		// dtor calls quit()
		EventLoopThread thr2;
		EventLoop* loop = thr2.startLoop();
		loop->runInLoop(std::bind(print, loop));
		sleepUsec(500 * 1000);
	}

	{
		// quit() before dtor
		EventLoopThread thr3;
		EventLoop* loop = thr3.startLoop();
		loop->runInLoop(std::bind(quit, loop));
		sleepUsec(500 * 1000);
	}
}