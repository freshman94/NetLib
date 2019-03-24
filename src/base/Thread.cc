#include <base/Thread.h>
#include <base/CurrentThread.h>
#include <log/Logger.h>

#include <type_traits>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/prctl.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <linux/unistd.h>

namespace CurrentThread
{
	__thread int t_cachedTid = 0;
	__thread char t_tidString[32];
	__thread int t_tidStringLength = 6;
	__thread const char* t_threadName = "unknown";
}

pid_t gettid(){
	return static_cast<pid_t>(syscall(SYS_gettid));
}

void CurrentThread::cacheTid(){
	if (t_cachedTid == 0){
		t_cachedTid = gettid();
		t_tidStringLength = snprintf(t_tidString, sizeof t_tidString, "%5d ", t_cachedTid);
	}
}

bool CurrentThread::isMainThread() {
	return tid() == getpid();
}


struct ThreadData{
	typedef Thread::ThreadFunc ThreadFunc;
	ThreadFunc func_;
	string name_;
	pid_t* tid_;
	CountDownLatch* latch_;

	ThreadData(ThreadFunc func,
		const string& name,
		pid_t* tid,
		CountDownLatch* latch)
		: func_(std::move(func)),
		name_(name),
		tid_(tid),
		latch_(latch)
	{ }

	void runInThread() {
		*tid_ = CurrentThread::tid();	//cache tid
		tid_ = NULL;
		latch_->countDown();
		latch_ = NULL;

		CurrentThread::t_threadName = name_.empty() ? "HttpServer_Thread" : name_.c_str();
		prctl(PR_SET_NAME, CurrentThread::t_threadName);

		func_();
		CurrentThread::t_threadName = "finished";
	}
};

void* startThread(void* obj)
{
	ThreadData* data = static_cast<ThreadData*>(obj);
	data->runInThread();
	delete data;
	return NULL;
}


Thread::Thread(ThreadFunc func, const string& name)
	: started_(false),
	joined_(false),
	pthreadId_(0),
	tid_(0),
	func_(std::move(func)),
	name_(name),
	latch_(1)
{
	setDefaultName();
}

Thread::~Thread(){
	if (started_ && !joined_)
		pthread_detach(pthreadId_);
}

void Thread::setDefaultName(){
	if (name_.empty()){
		char buf[32];
		snprintf(buf, sizeof buf, "HttpServer_Thread");
		name_ = buf;
	}
}

void Thread::start(){
	assert(!started_);
	started_ = true;

	ThreadData* data = new ThreadData(func_, name_, &tid_, &latch_);
	if (pthread_create(&pthreadId_, NULL, &startThread, data)){
		started_ = false;
		delete data;
		LOG_FATAL << "Failed in pthread_create";
	}
	//CountDownLatch是为了确保thread的func真正地启动起来，这时start函数才能返回。
	else{
		latch_.wait();
		assert(tid_ > 0);
	}
}

int Thread::join(){
	assert(started_);
	assert(!joined_);
	joined_ = true;
	return pthread_join(pthreadId_, NULL);
}
