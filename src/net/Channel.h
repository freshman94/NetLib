#pragma once

#include <base/noncopyable.h>

#include <functional>
#include <memory>

class EventLoop;

//ÿ��Channelֻ����һ��EventLoop������ע������Ӧ�¼���
class Channel : noncopyable
{
public:
	typedef std::function<void()> Callback;

	Channel(EventLoop* loop, int fd);
	~Channel();

	void handleEvent();
	void setReadCallback(Callback cb){ readCallback_ = std::move(cb);}
	void setWriteCallback(Callback cb){ writeCallback_ = std::move(cb);}
	void setCloseCallback(Callback cb){ closeCallback_ = std::move(cb);}
	void setErrorCallback(Callback cb){ errorCallback_ = std::move(cb);}

	//������ƣ���֤��Channel��handleEventʱ��Channel�������Ķ�������ɴ��
	void tie(const std::shared_ptr<void>&);

	int fd() const { return fd_; }
	__uint32_t events() const { return events_; }
	void set_events(__uint32_t revt) { events_ = revt; }
	void set_revents(__uint32_t revt) { revents_ = revt; }												
	bool isNoneEvent() const { return events_ == NoneEvent; }

	void enableReading() { events_ |= ReadEvent; update(); }
	void disableReading() { events_ &= ~ReadEvent; update(); }
	void enableWriting() { events_ |= WriteEvent; update(); }
	void disableWriting() { events_ &= ~WriteEvent; update(); }
	void disableAll() { events_ = NoneEvent; update(); }
	bool EnabledWriting() const { return events_ & WriteEvent; }
	bool EnabledReading() const { return events_ & ReadEvent; }

	//epoll
	int index() { return index_; }
	void set_index(int idx) { index_ = idx; }

	//log
	string reventsToString() const;
	string eventsToString() const;

	EventLoop* ownerLoop() { return loop_; }
	void remove();

private:
	static string eventsToString(int fd, __uint32_t ev);

	void update();
	void handleEventWithGuard();

	static const __uint32_t NoneEvent;
	static const __uint32_t ReadEvent;
	static const __uint32_t WriteEvent;

	EventLoop* loop_;
	const int  fd_;
	__uint32_t events_;	//���ĵ��¼�
	__uint32_t revents_; //Ŀǰ����¼�(received events)
	int        index_;

	std::weak_ptr<void> tie_;
	bool tied_;
	bool eventHandling_;
	bool addedToLoop_;
	Callback readCallback_;
	Callback writeCallback_;
	Callback closeCallback_;
	Callback errorCallback_;
};

