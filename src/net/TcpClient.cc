#include <net/TcpClient.h>
#include <log/Logger.h>
#include <net/Connector.h>
#include <net/EventLoop.h>
#include <net/SocketsOps.h>

#include <stdio.h>  // snprintf

void RemoveConnection(EventLoop* loop, const TcpConnectionPtr& conn)
{
	loop->queueInLoop(std::bind(&TcpConnection::connectDestroyed, conn));
}

extern void defaultConnectionCallback(const TcpConnectionPtr& conn);
extern void defaultMessageCallback(const TcpConnectionPtr&, Buffer* buf);

TcpClient::TcpClient(EventLoop* loop,
	const InetAddress& serverAddr,
	const string& nameArg)
	: loop_(loop),
	connector_(new Connector(loop, serverAddr)),
	name_(nameArg),
	connectionCallback_(defaultConnectionCallback),
	messageCallback_(defaultMessageCallback),
	retry_(false),
	connect_(true),
	nextConnId_(1)
{
	connector_->setNewConnectionCallback(
		std::bind(&TcpClient::newConnection, this, _1));
	LOG_INFO << "TcpClient::TcpClient[" << name_
		<< "] - connector ";
}

TcpClient::~TcpClient()
{
	LOG_INFO << "TcpClient::~TcpClient[" << name_
		<< "] - connector ";
	TcpConnectionPtr conn;
	bool unique = false;
	{
		MutexLockGuard lock(mutex_);
		unique = connection_.unique();
		conn = connection_;
	}
	if (conn)
	{
		assert(loop_ == conn->getLoop());
		Callback cb = std::bind(&RemoveConnection, loop_, _1);
		loop_->runInLoop(std::bind(&TcpConnection::setCloseCallback, conn, cb));
		if (unique)
		{
			conn->forceClose();
		}
	}
	else
		connector_->stop();
}

void TcpClient::connect()
{
	LOG_INFO << "TcpClient::connect[" << name_ << "] - connecting to "
		<< connector_->serverAddress().toIpPort();
	connect_ = true;
	connector_->start();
}

void TcpClient::disconnect()
{
	connect_ = false;

	{
		MutexLockGuard lock(mutex_);
		if (connection_)
			connection_->shutdown();
	}
}

void TcpClient::stop()
{
	connect_ = false;
	connector_->stop();
}

void TcpClient::newConnection(int sockfd)
{
	loop_->assertInLoopThread();
	InetAddress peerAddr(sockets::getPeerAddr(sockfd));
	char buf[32];
	snprintf(buf, sizeof buf, ":%s#%d", peerAddr.toIpPort().c_str(), nextConnId_);
	++nextConnId_;
	string connName = name_ + buf;

	InetAddress localAddr(sockets::getLocalAddr(sockfd));
	TcpConnectionPtr conn(new TcpConnection(loop_,
		connName, sockfd, localAddr, peerAddr));

	conn->setConnectionCallback(connectionCallback_);
	conn->setMessageCallback(messageCallback_);
	conn->setCloseCallback(std::bind(&TcpClient::removeConnection, this, _1));
	{
		MutexLockGuard lock(mutex_);
		connection_ = conn;
	}
	conn->connectEstablished();
}

void TcpClient::removeConnection(const TcpConnectionPtr& conn)
{
	loop_->assertInLoopThread();
	assert(loop_ == conn->getLoop());

	{
		MutexLockGuard lock(mutex_);
		assert(connection_ == conn);
		connection_.reset();
	}

	loop_->queueInLoop(std::bind(&TcpConnection::connectDestroyed, conn));
	if (retry_ && connect_)
	{
		LOG_INFO << "TcpClient::connect[" << name_ << "] - Reconnecting to "
			<< connector_->serverAddress().toIpPort();
		connector_->restart();
	}
}

