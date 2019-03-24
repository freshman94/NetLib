#include <log/Logger.h>
#include <net/Channel.h>
#include <net/EventLoop.h>

#include <sstream>
#include <sys/epoll.h>

const __uint32_t Channel::NoneEvent = 0;
const __uint32_t Channel::ReadEvent = EPOLLIN | EPOLLPRI;
const __uint32_t Channel::WriteEvent = EPOLLOUT;

Channel::Channel(EventLoop* loop, int fd__)
	: loop_(loop),
	fd_(fd__),
	events_(0),
	revents_(0),
	index_(-1),
	tied_(false),
	eventHandling_(false),
	addedToLoop_(false)
{}

//每个Channel只负责一个fd，但并不拥有这个fd，构造时不关闭这个fd
Channel::~Channel(){
	assert(!eventHandling_);
	assert(!addedToLoop_);
	if (loop_->isInLoopThread())
		assert(!loop_->hasChannel(this));
}

//保活机制，保证在Channel在handleEvent时，Channel所依赖的对象的依旧存活
//那么其所依赖的对象何时才会被析构？
void Channel::tie(const std::shared_ptr<void>& obj){													
	tie_ = obj;	
	tied_ = true;
}

void Channel::update(){
	addedToLoop_ = true;
	loop_->updateChannel(this);
}

void Channel::remove(){
	assert(isNoneEvent());
	addedToLoop_ = false;
	loop_->removeChannel(this);
}

void Channel::handleEvent(){
	std::shared_ptr<void> guard;
	if (tied_){//是否使用了保活机制，若使用了，判断Channel所依赖的对象是否还存活
		guard = tie_.lock();
		if (guard)
			handleEventWithGuard();
	}
	else
		handleEventWithGuard();
}

void Channel::handleEventWithGuard(){
	eventHandling_ = true;
	LOG_TRACE << reventsToString();
	if ((revents_ & EPOLLHUP) && !(revents_ & EPOLLIN))
		if (closeCallback_) closeCallback_();

	if (revents_ & EPOLLERR)
		if (errorCallback_) errorCallback_();

	if (revents_ & (EPOLLIN | EPOLLPRI | EPOLLRDHUP))
		if (readCallback_) readCallback_();

	if (revents_ & EPOLLOUT)
		if (writeCallback_) writeCallback_();

	eventHandling_ = false;
}

string Channel::reventsToString() const{
	return eventsToString(fd_, revents_);
}

string Channel::eventsToString() const{
	return eventsToString(fd_, events_);
}

string Channel::eventsToString(int fd, __uint32_t ev){
	std::ostringstream oss;
	oss << fd << ": ";
	if (ev & EPOLLIN)
		oss << "IN ";
	if (ev & EPOLLPRI)
		oss << "PRI ";
	if (ev & EPOLLOUT)
		oss << "OUT ";
	if (ev & EPOLLHUP)
		oss << "HUP ";
	if (ev & EPOLLRDHUP)
		oss << "RDHUP ";
	if (ev & EPOLLERR)
		oss << "ERR ";

	return oss.str();
}
