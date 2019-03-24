#include <net/EventLoop.h>
#include <log/Logger.h>
#include <base/Mutex.h>
#include <net/Channel.h>
#include <net/Epoll.h>
#include <net/SocketsOps.h>

#include <algorithm>
#include <signal.h>
#include <sys/eventfd.h>
#include <unistd.h>


__thread EventLoop* t_loopInThisThread = 0;

int createEventfd(){
	int evtfd = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
	if (evtfd < 0){
		LOG_ERROR << "Failed in eventfd";
		abort();
	}
	return evtfd;
}

class IgnoreSigPipe{
public:
	IgnoreSigPipe(){
		signal(SIGPIPE, SIG_IGN);
	}
};

IgnoreSigPipe initObj;



EventLoop* EventLoop::getEventLoopOfCurrentThread()
{
	return t_loopInThisThread;
}

EventLoop::EventLoop()
	: looping_(false),
	quit_(false),
	eventHandling_(false),
	callingPendingFunctors_(false),
	threadId_(CurrentThread::tid()),
	epoll_(new Epoll(this)),
	wakeupFd_(createEventfd()),
	wakeupChannel_(new Channel(this, wakeupFd_)),
	currentActiveChannel_(NULL)
{
	LOG_DEBUG << "EventLoop created " << this << " in thread " << threadId_;
	//ÿ���߳�ֻ����һ��EventLoop����
	if (t_loopInThisThread)
		LOG_FATAL << "Another EventLoop " << t_loopInThisThread
			<< " exists in this thread " << threadId_;
	else
		t_loopInThisThread = this;

	wakeupChannel_->setReadCallback(
		std::bind(&EventLoop::handleRead, this));
	//���л����update()����eventfd���뵽epollfd_�Ĺ�ע�¼��б���
	wakeupChannel_->enableReading();	
}

EventLoop::~EventLoop(){
	LOG_DEBUG << "EventLoop " << this << " of thread " << threadId_
		<< " destructs in thread " << CurrentThread::tid();
	wakeupChannel_->disableAll();
	wakeupChannel_->remove();
	close(wakeupFd_);
	t_loopInThisThread = NULL;
}

//epoll�����activeChannels)===>����activeChannels�е��¼�
//===>����pendingFunctors_�е�cb
void EventLoop::loop(){
	assert(!looping_);
	assertInLoopThread();
	looping_ = true;
	quit_ = false;
	LOG_TRACE << "EventLoop " << this << " start looping";

	while (!quit_){
		activeChannels_.clear();
		epoll_->epoll(&activeChannels_);
		eventHandling_ = true;
		for (Channel* channel : activeChannels_){
			currentActiveChannel_ = channel;
			currentActiveChannel_->handleEvent();
		}
		currentActiveChannel_ = NULL;
		eventHandling_ = false;
		ExecPendingFuncs();
	}
	LOG_TRACE << "EventLoop " << this << " stop looping";
	looping_ = false;
}

void EventLoop::quit(){
	quit_ = true;
	if (!isInLoopThread())
		wakeup();
}

/*
��EventLoop��IO�߳���ִ��ĳ���û�����ص�
��1�����û��ڵ�ǰIO�̵߳���runInLoop���ص�ͬ������
��2�����û��������̵߳���,cb�ᱻ������У�������IO�߳����������cb

****���������������̼߳���������ڲ�����������±�֤�̰߳�ȫ��****

*/
void EventLoop::runInLoop(Functor cb){
	if (isInLoopThread())
		cb();
	else
		queueInLoop(std::move(cb));
}

void EventLoop::queueInLoop(Functor cb)
{
	{
		MutexLockGuard lock(mutex_);
		pendingFunctors_.push_back(std::move(cb));
	}

	if (!isInLoopThread() || callingPendingFunctors_)
		wakeup();
}

size_t EventLoop::queueSize() const{
	MutexLockGuard lock(mutex_);
	return pendingFunctors_.size();
}

void EventLoop::updateChannel(Channel* channel){
	assert(channel->ownerLoop() == this);
	assertInLoopThread();
	epoll_->updateChannel(channel);
}

void EventLoop::removeChannel(Channel* channel){
	assert(channel->ownerLoop() == this);
	assertInLoopThread();
	if (eventHandling_){
		assert(currentActiveChannel_ == channel ||
			std::find(activeChannels_.begin(), activeChannels_.end(), channel) == activeChannels_.end());
	}
	epoll_->removeChannel(channel);
}

bool EventLoop::hasChannel(Channel* channel){
	assert(channel->ownerLoop() == this);
	assertInLoopThread();
	return epoll_->hasChannel(channel);
}

void EventLoop::abortNotInLoopThread(){
	LOG_FATAL << "EventLoop::abortNotInLoopThread - EventLoop " << this
		<< " was created in threadId_ = " << threadId_
		<< ", current thread id = " << CurrentThread::tid();
}

void EventLoop::wakeup(){
	uint64_t one = 1;
	ssize_t n = sockets::write(wakeupFd_, &one, sizeof one);
	if (n != sizeof one)
		LOG_ERROR << "EventLoop::wakeup() writes " << n << " bytes instead of 8";
}

//wakeupfd_��readCB
void EventLoop::handleRead(){
	uint64_t one = 1;
	ssize_t n = sockets::read(wakeupFd_, &one, sizeof one);
	if (n != sizeof one)
		LOG_ERROR << "EventLoop::handleRead() reads " << n << " bytes instead of 8";
}

void EventLoop::ExecPendingFuncs(){
	std::vector<Functor> functors;
	callingPendingFunctors_ = true;
	//functors�������Ǿ��������ٽ����ķ�Χ
	//�Ա����ٽ��������Ҳ���ʱ�ĺ���
	{
		MutexLockGuard lock(mutex_);
		functors.swap(pendingFunctors_);
	}
	for (const Functor& functor : functors)
		functor();
	callingPendingFunctors_ = false;
}

