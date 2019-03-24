#include <net/TcpServer.h>

#include <log/Logger.h>
#include <base/Thread.h>
#include <net/EventLoop.h>
#include <net/InetAddress.h>

#include <utility>
#include <stdio.h>
#include <unistd.h>

int numThreads = 0;

class EchoServer {
public:
	EchoServer(EventLoop* loop, const InetAddress& listenAddr)
		: loop_(loop),
		server_(loop, listenAddr, numThreads, "EchoServer")
	{
		server_.setConnEstabedCallback(
			std::bind(&EchoServer::onConnection, this, _1));
		server_.setMessageCallback(
			std::bind(&EchoServer::onMessage, this, _1, _2));
	}

	void start() {
		server_.start();
	}
	// void stop();

private:
	void onConnection(const TcpConnectionPtr& conn) {
		LOG_TRACE << conn->peerAddress().toIpPort() << " -> "
			<< conn->localAddress().toIpPort() << " is "
			<< (conn->connected() ? "UP" : "DOWN");

		conn->send("hello\n");
	}

	void onMessage(const TcpConnectionPtr& conn, Buffer* buf) {
		string msg(buf->retrieveAllAsString());
		LOG_TRACE << conn->name() << " recv " << msg.size() << " bytes";
		if (msg == "exit\n")
		{
			conn->send("bye\n");
			conn->shutdown();
		}
		if (msg == "quit\n")
			loop_->quit();
		conn->send(msg);
	}

	EventLoop* loop_;
	TcpServer server_;
};

int main(int argc, char* argv[]) {
	LOG_INFO << "pid = " << getpid() << ", tid = " << CurrentThread::tid();
	LOG_INFO << "sizeof TcpConnection = " << sizeof(TcpConnection);
	if (argc > 1)
		numThreads = atoi(argv[1]);
	EventLoop loop;
	InetAddress listenAddr(2000);
	EchoServer server(&loop, listenAddr);

	server.start();
	loop.loop();
}