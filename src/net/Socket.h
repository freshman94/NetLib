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

	//���ɹ�������һ��SOCK_NONBLOCK | SOCK_CLOEXEC�������׽��֡�������peeraddr
	//��ʧ�ܣ�����-1��ͬʱpeeraddrû�б��ı䡣
	int accept(InetAddress* peeraddr);

	void shutdownWrite();
	void setTcpNoDelay(bool on); //Enable/disable TCP_NODELAY
	void setReuseAddr(bool on);	// Enable/disable SO_REUSEADDR
	void setReusePort(bool on);	// Enable/disable SO_REUSEPORT
	void setKeepAlive(bool on); // Enable/disable SO_KEEPALIVE

private:
	const int sockfd_;
};