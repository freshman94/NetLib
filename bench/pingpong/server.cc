#include <net/TcpServer.h>
#include <base/Atomic.h>
#include <log/Logger.h>
#include <base/Thread.h>
#include <net/EventLoop.h>
#include <net/InetAddress.h>

#include <utility>
#include <stdio.h>
#include <unistd.h>

void onConnection(const TcpConnectionPtr& conn)
{
  if (conn->connected())
    conn->setTcpNoDelay(true);
}

void onMessage(const TcpConnectionPtr& conn, Buffer* buf)
{
  conn->send(buf);
}

int main(int argc, char* argv[])
{
  if (argc < 4)
  {
    fprintf(stderr, "Usage: server <address> <port> <threads>\n");
  }
  else
  {
    LOG_INFO << "pid = " << getpid() << ", tid = " << CurrentThread::tid();
    Logger::setLogLevel(Logger::WARN);

    const char* ip = argv[1];
    uint16_t port = static_cast<uint16_t>(atoi(argv[2]));
    InetAddress listenAddr(ip, port);
    int threadCount = atoi(argv[3]);

	if (threadCount <= 1)	threadCount = 0;

    EventLoop loop;

    TcpServer server(&loop, listenAddr, "PingPong", threadCount);

    server.setConnectionCallback(onConnection);
    server.setMessageCallback(onMessage);

    server.start();

    loop.loop();
  }
}

