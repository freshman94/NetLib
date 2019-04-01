#include <net/Connector.h>
#include <log/Logger.h>
#include <net/Channel.h>
#include <net/EventLoop.h>
#include <net/SocketsOps.h>

#include <errno.h>

const int Connector::MaxRetryDelayMs;

Connector::Connector(EventLoop* loop, const InetAddress& serverAddr)
	: loop_(loop),
	serverAddr_(serverAddr),
	connect_(false),
	state_(Disconnected),
	retryDelayMs_(InitRetryDelayMs)
{
	LOG_DEBUG << "ctor[" << this << "]";
}

Connector::~Connector()
{
	LOG_DEBUG << "dtor[" << this << "]";
	assert(!channel_);
}

void Connector::start()
{
	connect_ = true;
	loop_->runInLoop(std::bind(&Connector::startInLoop, this));
}

void Connector::startInLoop()
{
	loop_->assertInLoopThread();
	assert(state_ == Disconnected);
	if (connect_)
		connect();
	else
		LOG_DEBUG << "do not connect";
}

void Connector::stop()
{
	connect_ = false;
	loop_->queueInLoop(std::bind(&Connector::stopInLoop, this));
}

void Connector::stopInLoop()
{
	loop_->assertInLoopThread();
	if (state_ == Connecting)
	{
		setState(Disconnected);
		int sockfd = removeAndResetChannel();
		retry(sockfd);
	}
}

void Connector::connect()
{
	int sockfd = sockets::createNonblockingOrDie(serverAddr_.family());
	int ret = sockets::connect(sockfd, serverAddr_.getSockAddr());
	int savedErrno = (ret == 0) ? 0 : errno;
	switch (savedErrno)
	{
	case 0:
	case EINPROGRESS:
	case EINTR:
	case EISCONN:
		connecting(sockfd);
		break;

	case EAGAIN:
	case EADDRINUSE:
	case EADDRNOTAVAIL:
	case ECONNREFUSED:
	case ENETUNREACH:
		retry(sockfd);
		break;

	case EACCES:
	case EPERM:
	case EAFNOSUPPORT:
	case EALREADY:
	case EBADF:
	case EFAULT:
	case ENOTSOCK:
		LOG_ERROR << "connect error in Connector::startInLoop " << savedErrno;
		sockets::close(sockfd);
		break;

	default:
		LOG_ERROR << "Unexpected error in Connector::startInLoop " << savedErrno;
		sockets::close(sockfd);
		break;
	}
}

void Connector::restart()
{
	loop_->assertInLoopThread();
	setState(Disconnected);
	retryDelayMs_ = InitRetryDelayMs;
	connect_ = true;
	startInLoop();
}

void Connector::connecting(int sockfd)
{
	setState(Connecting);
	assert(!channel_);
	channel_.reset(new Channel(loop_, sockfd));
	channel_->setWriteCallback(std::bind(&Connector::handleWrite, this));
	channel_->setErrorCallback(std::bind(&Connector::handleError, this));
	channel_->enableWriting();
}

int Connector::removeAndResetChannel()
{
	channel_->disableAll();
	channel_->remove();
	int sockfd = channel_->fd();
	loop_->queueInLoop(std::bind(&Connector::resetChannel, this));
	return sockfd;
}

void Connector::resetChannel()
{
	channel_.reset();
}

void Connector::handleWrite()
{
	LOG_TRACE << "Connector::handleWrite " << state_;

	if (state_ == Connecting)
	{
		//一次连接，channel_只使用一次，用于设置IO回调。
		int sockfd = removeAndResetChannel();
		int err = sockets::getSocketError(sockfd);
		if (err)
		{
			LOG_WARN << "Connector::handleWrite - SO_ERROR = "
				<< err << " " << strerror(err);
			retry(sockfd);
		}
		else if (sockets::isSelfConnect(sockfd))
		{
			LOG_WARN << "Connector::handleWrite - Self connect";
			retry(sockfd);
		}
		else
		{
			setState(Connected);
			if (connect_)
			{
				newConnectionCallback_(sockfd);
			}
			else
			{
				sockets::close(sockfd);
			}
		}
	}
	else
	{
		assert(state_ == Disconnected);
	}
}

void Connector::handleError()
{
	LOG_ERROR << "Connector::handleError state=" << state_;
	if (state_ == Connecting)
	{
		int sockfd = removeAndResetChannel();
		int err = sockets::getSocketError(sockfd);
		LOG_TRACE << "SO_ERROR = " << err << " " << strerror(err);
		retry(sockfd);
	}
}

void Connector::retry(int sockfd)
{
	sockets::close(sockfd);
	setState(Disconnected);
	if (connect_)
	{
		LOG_INFO << "Connector::retry - Retry connecting to " << serverAddr_.toIpPort()
			<< " in " << retryDelayMs_ << " milliseconds. ";
		loop_->runAfter(retryDelayMs_ / 1000.0,
			std::bind(&Connector::startInLoop, shared_from_this()));
		retryDelayMs_ = std::min(retryDelayMs_ * 2, MaxRetryDelayMs);
	}
	else
		LOG_DEBUG << "do not connect";
}

