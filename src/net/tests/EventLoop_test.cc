#include <assert.h>
#include <stdio.h>
#include <unistd.h> 

#include <net/EventLoop.h>
#include <base/Thread.h>
EventLoop* g_loop;

void callback(){
	printf("callback(): pid = %d, tid = %d\n", getpid(), CurrentThread::tid());
	EventLoop anotherLoop;//一个线程有两个EventLoop，会abort
	//g_loop->quit();
}

void threadFunc(){
	printf("threadFunc(): pid = %d, tid = %d\n", getpid(), CurrentThread::tid());
	assert(EventLoop::getEventLoopOfCurrentThread() == NULL);
	EventLoop loop;
	assert(EventLoop::getEventLoopOfCurrentThread() == &loop);
	loop.runInLoop(callback);
	loop.loop();
}
int main(){
	printf("main(): pid = %d, tid = %d\n", getpid(), CurrentThread::tid());
	assert(EventLoop::getEventLoopOfCurrentThread() == NULL);
	EventLoop loop;
	g_loop = &loop;
	assert(EventLoop::getEventLoopOfCurrentThread() == &loop);
	Thread thread(threadFunc);
	thread.start();
	loop.loop();
}