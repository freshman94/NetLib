#pragma once

#include <net/EventLoop.h>
#include <net/Channel.h>

#include <vector>
#include <map>

struct epoll_event;

//封装epoll
//epoll_event.data.ptr为Channel类型,Channel的更新和删除在此实现
class Epoll{
public:
	Epoll(EventLoop* loop);
	~Epoll();

	typedef std::vector<Channel*> ChannelList;
	void epoll(ChannelList* activeChannels);

	void updateChannel(Channel* channel);
	void removeChannel(Channel* channel);
	bool hasChannel(Channel* channel) const;

	void assertInLoopThread() const{
		ownerLoop_->assertInLoopThread();
	}

private:
	typedef std::map<int, Channel*> ChannelMap;
	typedef std::vector<struct epoll_event> EventList;
	

	static const int InitEventListSize = 16;
	static const char* operationToString(int op);	//log

	void fillActiveChannels(int numEvents,
		ChannelList* activeChannels) const;	//epoll调用，获取待处理事件的Channels
	void update(int operation, Channel* channel);

	//<fd, Channel*>: 快速定位当前fd是New，Added或Deleted，
	//并对应地调用EPOLL_CTL_ADD、EPOLL_CTL_MOD或EPOLL_CTL_DEL
	ChannelMap channels_; 
	EventLoop* ownerLoop_;
	int epollfd_;
	EventList events_;	//待处理的事件的数组
};

