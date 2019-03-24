#pragma once

#include <base/Types.h>
#include <net/TcpConnection.h>

#include <map>

class EventLoop;
class EventLoopThreadPool;

using std::placeholders::_1;
using std::placeholders::_2;

class TcpServer : noncopyable{
public:
	typedef std::function<void(int sockfd, const InetAddress&)> NewConnectionCallback;

	TcpServer(EventLoop* loop,const InetAddress& listenAddr,
		int numThreads = 0, const string& nameArg = string());
	~TcpServer();

	const string& ipPort() const { return ipPort_; }
	const string& name() const { return name_; }
	EventLoop* getLoop() const { return loop_; }

	void setConnEstabedCallback(const Callback& cb) { connEstabedCallback_ = cb; }
	void setNewConnectionCallback(const NewConnectionCallback& cb){ newConnectionCallback_ = cb;}
	void setMessageCallback(const MessageCallback& cb) { messageCallback_ = cb; }

	std::shared_ptr<EventLoopThreadPool> threadPool(){ return threadPool_;}
	void start();

private:
	void ReadCallback();
	void listen();
	void newConnection(int sockfd, const InetAddress& peerAddr);
	void removeConnection(const TcpConnectionPtr& conn);
	void removeConnectionInLoop(const TcpConnectionPtr& conn);

	typedef std::map<string, TcpConnectionPtr> ConnectionMap;
	EventLoop* loop_;
	const string ipPort_;
	const string name_;
	bool listenning_;
	bool started_;
	Socket listenSocket_;
	Channel listenChannel_;
	std::shared_ptr<EventLoopThreadPool> threadPool_;
	Callback connEstabedCallback_;
	NewConnectionCallback newConnectionCallback_;
	MessageCallback messageCallback_;
	ConnectionMap connections_;
	int nextConnId_;
};
