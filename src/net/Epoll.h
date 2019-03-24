#pragma once

#include <net/EventLoop.h>
#include <net/Channel.h>

#include <vector>
#include <map>

struct epoll_event;

//��װepoll
//epoll_event.data.ptrΪChannel����,Channel�ĸ��º�ɾ���ڴ�ʵ��
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
		ChannelList* activeChannels) const;	//epoll���ã���ȡ�������¼���Channels
	void update(int operation, Channel* channel);

	//<fd, Channel*>: ���ٶ�λ��ǰfd��New��Added��Deleted��
	//����Ӧ�ص���EPOLL_CTL_ADD��EPOLL_CTL_MOD��EPOLL_CTL_DEL
	ChannelMap channels_; 
	EventLoop* ownerLoop_;
	int epollfd_;
	EventList events_;	//��������¼�������
};

