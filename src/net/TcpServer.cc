#include <net/TcpServer.h>
#include <log/Logger.h>
#include <net/EventLoop.h>
#include <net/EventLoopThreadPool.h>
#include <net/SocketsOps.h>
#include <net/Buffer.h>

#include <stdio.h>  // snprintf


void defaultConnEstabedCallback(const TcpConnectionPtr& conn) {
	LOG_TRACE << conn->localAddress().toIpPort() << " -> "
		<< conn->peerAddress().toIpPort() << " is "
		<< (conn->connected() ? "UP" : "DOWN");
}

void defaultMessageCallback(const TcpConnectionPtr&, Buffer* buf){
	buf->retrieveAll();
}

TcpServer::TcpServer(EventLoop* loop,const InetAddress& listenAddr,
	int numThreads, const string& nameArg)
	: loop_(loop),
	ipPort_(listenAddr.toIpPort()),
	name_(nameArg),
	listenning_(false),
	started_(false),
	listenSocket_(sockets::createNonblockingOrDie(listenAddr.family())),
	listenChannel_(loop, listenSocket_.fd()),
	threadPool_(new EventLoopThreadPool(loop, name_, numThreads)),
	connEstabedCallback_(defaultConnEstabedCallback),
	messageCallback_(defaultMessageCallback),
	nextConnId_(0)
{
	listenSocket_.setReuseAddr(true);
	listenSocket_.setReusePort(true);
	listenSocket_.bindAddress(listenAddr);
	listenChannel_.setReadCallback(std::bind(&TcpServer::ReadCallback, this));
	setNewConnectionCallback(std::bind(&TcpServer::newConnection, this, _1, _2));
}

TcpServer::~TcpServer(){
	loop_->assertInLoopThread();
	LOG_TRACE << "TcpServer::~TcpServer [" << name_ << "] destructing";

	for (auto& item : connections_){
		TcpConnectionPtr conn(item.second);
		item.second.reset();
		conn->getLoop()->runInLoop(
			std::bind(&TcpConnection::connectDestroyed, conn));
	}
}

void TcpServer::start(){
	assert(!started_);
	started_ = true;
	threadPool_->start();
	assert(!listenning_);
	loop_->runInLoop(std::bind(&TcpServer::listen, this));
}

void TcpServer::listen(){
	loop_->assertInLoopThread();
	listenning_ = true;
	listenSocket_.listen();
	listenChannel_.enableReading();
}

void TcpServer::ReadCallback(){
	loop_->assertInLoopThread();
	InetAddress peerAddr;
	int connfd = listenSocket_.accept(&peerAddr);
	if (connfd >= 0)
		newConnectionCallback_(connfd, peerAddr);
	else
		LOG_ERROR << "in TcpServer::ReadCallback";
}


void TcpServer::newConnection(int sockfd, const InetAddress& peerAddr){
	loop_->assertInLoopThread();
	EventLoop* ioLoop = threadPool_->getNextLoop();
	char buf[64];
	snprintf(buf, sizeof buf, "-%s#%d", ipPort_.c_str(), nextConnId_);
	++nextConnId_;
	string connName = name_ + buf;

	LOG_INFO << "TcpServer::newConnection [" << name_
		<< "] - new connection [" << connName
		<< "] from " << peerAddr.toIpPort();
	InetAddress localAddr(sockets::getLocalAddr(sockfd));
	TcpConnectionPtr conn(new TcpConnection(ioLoop,connName,
		sockfd,localAddr,peerAddr));
	connections_[connName] = conn;
	conn->setConnEstabedCallback(connEstabedCallback_);
	conn->setMessageCallback(messageCallback_);
	conn->setConnClosedCallback(std::bind(&TcpServer::removeConnection, this, _1));
	ioLoop->runInLoop(std::bind(&TcpConnection::connectEstablished, conn));
}

void TcpServer::removeConnection(const TcpConnectionPtr& conn){
	loop_->runInLoop(std::bind(&TcpServer::removeConnectionInLoop, this, conn));
}

void TcpServer::removeConnectionInLoop(const TcpConnectionPtr& conn){
	loop_->assertInLoopThread();
	LOG_INFO << "TcpServer::removeConnectionInLoop [" << name_
		<< "] - connection " << conn->name();
	connections_.erase(conn->name());
	EventLoop* ioLoop = conn->getLoop();
	ioLoop->queueInLoop(std::bind(&TcpConnection::connectDestroyed, conn));
}



