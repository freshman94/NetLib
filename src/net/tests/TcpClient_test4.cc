#include <net/EventLoop.h>
#include <net/TcpClient.h>
#include <log/Logger.h>


int main() {
	Logger::setLogLevel(Logger::TRACE);
	EventLoop loop;
	InetAddress serverAddr("127.0.0.1", 1234);
	TcpClient client(&loop, serverAddr, "TcpClient");
	client.connect();
	loop.loop();
}