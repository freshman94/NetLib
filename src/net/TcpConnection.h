#pragma once

#include <base/noncopyable.h>
#include <base/StringPiece.h>
#include <base/Types.h>
#include <net/Buffer.h>
#include <net/InetAddress.h>
#include <net/Socket.h>
#include <net/Channel.h>

#include <memory>
#include <boost/any.hpp>

class Channel;
class EventLoop;
class Socket;
class TcpConnection;

typedef std::shared_ptr<TcpConnection> TcpConnectionPtr;
typedef std::function<void(const TcpConnectionPtr&)> Callback;
typedef std::function<void(const TcpConnectionPtr&, Buffer*)> MessageCallback;

class TcpConnection : noncopyable,
	public std::enable_shared_from_this<TcpConnection>{
public:
	TcpConnection(EventLoop* loop, const string& name, int sockfd, 
		const InetAddress& localAddr,const InetAddress& peerAddr);
	~TcpConnection();

	EventLoop* getLoop() const { return loop_; }
	const string& name() const { return name_; }
	const InetAddress& localAddress() const { return localAddr_; }
	const InetAddress& peerAddress() const { return peerAddr_; }
	bool connected() const { return state_ == Connected; }
	bool disconnected() const { return state_ == Disconnected; }

	void send(const void* message, int len);
	void send(const StringPiece& message);
	void send(Buffer* message);

	void shutdown(); 
	void forceClose();
	void setTcpNoDelay(bool on);

	void setContext(const boost::any& context){ context_ = context;}
	const boost::any& getContext() const{ return context_;}
	boost::any* getMutableContext(){ return &context_;}

	void setConnectionCallback(const Callback& cb){ connectionCallback_ = cb;}
	void setCloseCallback(const Callback& cb) { closeCallback_ = cb; }
	void setMessageCallback(const MessageCallback& cb){ messageCallback_ = cb;}

	Buffer* inputBuffer(){ return &inputBuffer_;}
	Buffer* outputBuffer(){ return &outputBuffer_;}

	void connectEstablished();
	void connectDestroyed();  

private:
	enum StateE { Disconnected, Connecting, Connected, Disconnecting };
	void handleRead();
	void handleWrite();
	void handleClose();
	void handleError();
	void forceCloseInLoop();

	void setState(StateE s) { state_ = s; }
	const char* stateToString() const;
	void shutdownInLoop();
	void sendInLoop(const StringPiece& message);
	void sendInLoop(const void* message, size_t len);

	EventLoop* loop_;
	const string name_;
	StateE state_;  
	bool reading_;
	
	std::unique_ptr<Socket> socket_;
	std::unique_ptr<Channel> channel_;
	const InetAddress localAddr_;
	const InetAddress peerAddr_;
	Callback connectionCallback_;
	Callback closeCallback_;
	MessageCallback messageCallback_;
	
	Buffer inputBuffer_;
	Buffer outputBuffer_;
	boost::any context_;
};



