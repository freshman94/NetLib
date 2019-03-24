#include <net/EventLoopThreadPool.h>
#include <net/EventLoop.h>
#include <base/Thread.h>

#include <stdio.h>
#include <unistd.h>

void print(EventLoop* p = NULL) {
	printf("main(): pid = %d, tid = %d, loop = %p\n",
		getpid(), CurrentThread::tid(), p);
}

void init(EventLoop* p) {
	printf("init(): pid = %d, tid = %d, loop = %p\n",
		getpid(), CurrentThread::tid(), p);
}

int main() {
	print();

	EventLoop loop;
	loop.runInLoop(std::bind(&EventLoop::quit, &loop));

	{
		printf("Single thread %p:\n", &loop);
		EventLoopThreadPool model(&loop, "single", 0);
		model.start();
		EventLoop* nextLoop = model.getNextLoop();
		nextLoop->runInLoop(std::bind(init, nextLoop));
		assert(model.getNextLoop() == &loop);
		assert(model.getNextLoop() == &loop);
		assert(model.getNextLoop() == &loop);
	}

	{
		printf("Another thread:\n");
		EventLoopThreadPool model(&loop, "another", 1);
		model.start();
		EventLoop* nextLoop = model.getNextLoop();
		nextLoop->runInLoop(std::bind(print, nextLoop));
		assert(nextLoop != &loop);
		assert(nextLoop == model.getNextLoop());
		assert(nextLoop == model.getNextLoop());
		sleep(3);
	}

	{
		printf("Three threads:\n");
		EventLoopThreadPool model(&loop, "three", 3);
		model.start();
		EventLoop* nextLoop = model.getNextLoop();
		nextLoop->runInLoop(std::bind(print, nextLoop));
		assert(nextLoop != &loop);
		assert(nextLoop != model.getNextLoop());
		assert(nextLoop != model.getNextLoop());
		assert(nextLoop == model.getNextLoop());
	}

	loop.loop();
}