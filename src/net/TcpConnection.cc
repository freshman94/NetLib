#include <net/TcpConnection.h>
#include <log/Logger.h>
#include <net/Channel.h>
#include <net/EventLoop.h>
#include <net/Socket.h>
#include <net/SocketsOps.h>

#include <errno.h>

TcpConnection::TcpConnection(EventLoop* loop,
	const string& nameArg, int sockfd,
	const InetAddress& localAddr,
	const InetAddress& peerAddr)
	: loop_(loop), name_(nameArg),
	state_(Connecting), reading_(true),
	socket_(new Socket(sockfd)),
	channel_(new Channel(loop, sockfd)),
	localAddr_(localAddr),
	peerAddr_(peerAddr)
{
	channel_->setReadCallback(std::bind(&TcpConnection::handleRead, this));
	channel_->setWriteCallback(std::bind(&TcpConnection::handleWrite, this));
	channel_->setCloseCallback(std::bind(&TcpConnection::handleClose, this));
	channel_->setErrorCallback(std::bind(&TcpConnection::handleError, this));
	LOG_DEBUG << "TcpConnection::ctor[" << name_ << "] at " << this
		<< " fd=" << sockfd;
	socket_->setKeepAlive(true);
}

TcpConnection::~TcpConnection()
{
	LOG_DEBUG << "TcpConnection::dtor[" << name_ << "] at " << this
		<< " fd=" << channel_->fd();
	assert(state_ == Disconnected);
}

//发送数据
void TcpConnection::sendInLoop(const void* data, size_t len) {
	loop_->assertInLoopThread();
	ssize_t nwrote = 0;
	size_t remaining = len;
	bool faultError = false;

	// 当前没有发送数据或者输出缓冲区为空，直接发送数据
	if (!channel_->EnabledWriting() && outputBuffer_.readableBytes() == 0){
		nwrote = sockets::write(channel_->fd(), data, len);
		if (nwrote >= 0) {
			remaining = len - nwrote;
		}
		else{
			nwrote = 0;
			if (errno != EWOULDBLOCK){
				LOG_ERROR << "TcpConnection::sendInLoop";
				if (errno == EPIPE || errno == ECONNRESET)	//socket已失效
					faultError = true;
			}
		}
	}
	//将剩余数据存入缓冲区
	if (!faultError && remaining > 0){
		outputBuffer_.append(static_cast<const char*>(data) + nwrote, remaining);
		if (!channel_->EnabledWriting())	//关注writable事件
			channel_->enableWriting();
	}
}

void TcpConnection::sendInLoop(const StringPiece& message){
	sendInLoop(message.data(), message.size());
}

void TcpConnection::send(const void* data, int len){
	send(StringPiece(static_cast<const char*>(data), len));
}

void TcpConnection::send(const StringPiece& message){
	if (loop_->isInLoopThread())
		sendInLoop(message);
	else{
		void (TcpConnection::*fp)(const StringPiece& message) = &TcpConnection::sendInLoop;
		loop_->runInLoop(std::bind(fp, this, message.as_string()));
	}
}

void TcpConnection::send(Buffer* buf){
	if (loop_->isInLoopThread()){
		sendInLoop(buf->readablePtr(), buf->readableBytes());
		buf->retrieveAll();
	}
	else{
		void (TcpConnection::*fp)(const StringPiece& message) = &TcpConnection::sendInLoop;
		loop_->runInLoop(std::bind(fp, this, buf->retrieveAllAsString()));
	}
}

void TcpConnection::shutdown(){
	if (state_ == Connected){
		setState(Disconnecting);
		loop_->runInLoop(std::bind(&TcpConnection::shutdownInLoop, this));
	}
}

void TcpConnection::shutdownInLoop(){
	loop_->assertInLoopThread();
	if (!channel_->EnabledWriting())
		socket_->shutdownWrite();
}

void TcpConnection::setTcpNoDelay(bool on){
	socket_->setTcpNoDelay(on);
}

void TcpConnection::connectEstablished(){
	loop_->assertInLoopThread();
	assert(state_ == Connecting);
	setState(Connected);
	channel_->tie(shared_from_this());	//保活TcpConnection
	channel_->enableReading();
	if(connEstabedCallback_)
		connEstabedCallback_(shared_from_this());
}

void TcpConnection::connectDestroyed(){
	loop_->assertInLoopThread();
	if (state_ == Connected){
		setState(Disconnected);
		channel_->disableAll();
		if(connClosedCallback_)
			connClosedCallback_(shared_from_this());
	}
	channel_->remove();
}

void TcpConnection::handleRead(){
	loop_->assertInLoopThread();
	int savedErrno = 0;
	ssize_t n = inputBuffer_.readFd(channel_->fd(), &savedErrno);
	if (n > 0)
		messageCallback_(shared_from_this(), &inputBuffer_);
	else if (n == 0)
		handleClose();
	else{
		errno = savedErrno;
		LOG_ERROR << "TcpConnection::handleRead";
		handleError();
	}
}

void TcpConnection::handleWrite(){
	loop_->assertInLoopThread();
	if (channel_->EnabledWriting()){
		ssize_t n = sockets::write(channel_->fd(),
			outputBuffer_.readablePtr(),
			outputBuffer_.readableBytes());
		if (n > 0){
			outputBuffer_.retrieve(n);
			if (outputBuffer_.readableBytes() == 0){
				channel_->disableWriting();
				if (state_ == Disconnecting) {
					shutdownInLoop();
				}
			}
		}
		else
			LOG_ERROR << "TcpConnection::handleWrite";
	}
}

void TcpConnection::handleClose(){
	loop_->assertInLoopThread();
	LOG_TRACE << "fd = " << channel_->fd();
	assert(state_ == Connected || state_ == Disconnecting);
	setState(Disconnected);
	channel_->disableAll();

	if(connClosedCallback_)
		connClosedCallback_(shared_from_this());
}

void TcpConnection::handleError(){
	int err = sockets::getSocketError(channel_->fd());
	LOG_ERROR << "TcpConnection::handleError [" << name_
		<< "] - SO_ERROR = " << err << " " << strerror(err);
}

