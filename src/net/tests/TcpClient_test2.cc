// TcpClient destructs when TcpConnection is connected but unique.

#include <log/Logger.h>
#include <base/Thread.h>
#include <net/EventLoop.h>
#include <net/TcpClient.h>

const int MicroSecsPerSec = 1000 * 1000;

void sleepUsec(int64_t usec) {
	struct timespec ts = { 0, 0 };
	ts.tv_sec = static_cast<time_t>(usec / MicroSecsPerSec);
	ts.tv_nsec = static_cast<long>(usec % MicroSecsPerSec * 1000);
	nanosleep(&ts, NULL);
}

void threadFunc(EventLoop* loop)
{
  InetAddress serverAddr("127.0.0.1", 1234); // should succeed
  TcpClient client(loop, serverAddr, "TcpClient");
  client.connect();

  sleepUsec(1000*1000);
  // client destructs when connected.
}

int main(int argc, char* argv[])
{
  Logger::setLogLevel(Logger::DEBUG);

  EventLoop loop;
  loop.runAfter(3.0, std::bind(&EventLoop::quit, &loop));
  Thread thr(std::bind(threadFunc, &loop));
  thr.start();
  loop.loop();
}
