// TcpClient::stop() called in the same iteration of IO event

#include <log/Logger.h>
#include <net/EventLoop.h>
#include <net/TcpClient.h>

TcpClient* g_client;

void timeout()
{
  LOG_INFO << "timeout";
  g_client->stop();
}

const int MicroSecsPerSec = 1000 * 1000;

void sleepUsec(int64_t usec) {
	struct timespec ts = { 0, 0 };
	ts.tv_sec = static_cast<time_t>(usec / MicroSecsPerSec);
	ts.tv_nsec = static_cast<long>(usec % MicroSecsPerSec * 1000);
	nanosleep(&ts, NULL);
}

int main(int argc, char* argv[])
{
  EventLoop loop;
  InetAddress serverAddr("127.0.0.1", 2); // no such server
  TcpClient client(&loop, serverAddr, "TcpClient");
  g_client = &client;
  loop.runAfter(0.0, timeout);
  loop.runAfter(1.0, std::bind(&EventLoop::quit, &loop));
  client.connect();
  sleepUsec(100 * 1000);
  loop.loop();
}
