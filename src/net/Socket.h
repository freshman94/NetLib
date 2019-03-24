#pragma once

#include <base/noncopyable.h>

struct tcp_info;
class InetAddress;

class Socket : noncopyable{
public:
	explicit Socket(int sockfd)
		: sockfd_(sockfd) {}
	~Socket();

	int fd() const { return sockfd_; }
	
	bool getTcpInfo(struct tcp_info*) const;
	bool getTcpInfoString(char* buf, int len) const;

	//bindOrDie
	void bindAddress(const InetAddress& localaddr);
	//listernOrDie
	void listen();

	//若成功，返回一个SOCK_NONBLOCK | SOCK_CLOEXEC已连接套接字。并设置peeraddr
	//若失败，返回-1，同时peeraddr没有被改变。
	int accept(InetAddress* peeraddr);

	void shutdownWrite();
	void setTcpNoDelay(bool on); //Enable/disable TCP_NODELAY
	void setReuseAddr(bool on);	// Enable/disable SO_REUSEADDR
	void setReusePort(bool on);	// Enable/disable SO_REUSEPORT
	void setKeepAlive(bool on); // Enable/disable SO_KEEPALIVE

private:
	const int sockfd_;
};