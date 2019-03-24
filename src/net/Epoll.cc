#include <net/Epoll.h>
#include <log/Logger.h>

#include <assert.h>
#include <errno.h>
#include <poll.h>
#include <sys/epoll.h>
#include <unistd.h>

const int New = -1;
const int Added = 1;
const int Deleted = 2;

const int EpollTimeMs = 10000;	//epoll��ʱ��Ϊ10S

Epoll::Epoll(EventLoop* loop)
	: ownerLoop_(loop),
	epollfd_(epoll_create1(EPOLL_CLOEXEC)),
	events_(InitEventListSize)
{
	if (epollfd_ < 0)
		LOG_FATAL << "Epoll::Epoll";
}

Epoll::~Epoll(){
	close(epollfd_);
}

//epoll_wait������received events��Channel����activeChannels
void Epoll::epoll(ChannelList* activeChannels){
	LOG_TRACE << "fd total count " << channels_.size();
	int numEvents = epoll_wait(epollfd_, &*events_.begin(),
		static_cast<int>(events_.size()), EpollTimeMs);
	int savedErrno = errno;
	if (numEvents > 0){
		LOG_TRACE << numEvents << " events happened";
		fillActiveChannels(numEvents, activeChannels);
		if (implicit_cast<size_t>(numEvents) == events_.size()) {	//events_��̬����
			events_.resize(events_.size() * 2);
		}
	}
	else if (numEvents == 0) {
		LOG_TRACE << "nothing happened";
	}	
	else{
		//�������󣨺���EINTR��
		if (savedErrno != EINTR){
			errno = savedErrno;
			LOG_ERROR << "Epoll::epoll()";
		}
	}
}

//epoll_event.data.ptrΪChannel����
void Epoll::fillActiveChannels(int numEvents,
	ChannelList* activeChannels) const{
	assert(implicit_cast<size_t>(numEvents) <= events_.size());
	for (int i = 0; i < numEvents; ++i){
		Channel* channel = static_cast<Channel*>(events_[i].data.ptr);
		channel->set_revents(events_[i].events);
		activeChannels->push_back(channel);
	}
}

//ά��channels_���飬������upate����ִ����Ӧ��epoll_ctl����
void Epoll::updateChannel(Channel* channel){
	assertInLoopThread();
	const int index = channel->index();
	LOG_TRACE << "fd = " << channel->fd()
		<< " events = " << channel->events() << " index = " << index;
	if (index == New || index == Deleted){
		int fd = channel->fd();
		if (index == New){	//New Channel
			assert(channels_.find(fd) == channels_.end());
			channels_[fd] = channel;
		}
		else{ //Deleted Channel
			assert(channels_.find(fd) != channels_.end());
			assert(channels_[fd] == channel);
		}

		channel->set_index(Added);
		update(EPOLL_CTL_ADD, channel);
	}
	else{ //�Ѵ��ڵ�channel(Added)
		int fd = channel->fd();
		(void)fd;
		assert(channels_.find(fd) != channels_.end());
		assert(channels_[fd] == channel);
		assert(index == Added);
		//��Channelû�й�ע�κ��¼��������epollfd_���ĵ��¼���ɾ���������ΪDeleted
		//��ʵ�ʲ�δ��channels_ɾ��
		if (channel->isNoneEvent()){	
			update(EPOLL_CTL_DEL, channel);
			channel->set_index(Deleted);
		}
		else
			update(EPOLL_CTL_MOD, channel);
	}
}

void Epoll::removeChannel(Channel* channel){
	assertInLoopThread();
	int fd = channel->fd();
	LOG_TRACE << "fd = " << fd;
	assert(channels_.find(fd) != channels_.end());
	assert(channels_[fd] == channel);
	assert(channel->isNoneEvent());
	int index = channel->index();
	assert(index == Added|| index == Deleted);
	channels_.erase(fd);

	if (index == Added)
		update(EPOLL_CTL_DEL, channel);
	channel->set_index(New);
}

void Epoll::update(int operation, Channel* channel){
	struct epoll_event event;
	memZero(&event, sizeof event);
	event.events = channel->events();
	event.data.ptr = channel;
	int fd = channel->fd();
	LOG_TRACE << "epoll_ctl op = " << operationToString(operation)
		<< " fd = " << fd << " event = { " << channel->eventsToString() << " }";
	if (epoll_ctl(epollfd_, operation, fd, &event) < 0){
		if (operation == EPOLL_CTL_DEL)
			LOG_ERROR << "epoll_ctl op =" << operationToString(operation) << " fd =" << fd;
		else
			LOG_FATAL << "epoll_ctl op =" << operationToString(operation) << " fd =" << fd;
	}
}

bool Epoll::hasChannel(Channel* channel) const{
	assertInLoopThread();
	ChannelMap::const_iterator it = channels_.find(channel->fd());
	return it != channels_.end() && it->second == channel;
}

const char* Epoll::operationToString(int op){
	switch (op)
	{
	case EPOLL_CTL_ADD:
		return "ADD";
	case EPOLL_CTL_DEL:
		return "DEL";
	case EPOLL_CTL_MOD:
		return "MOD";
	default:
		return "Unknown Operation";
	}
}
