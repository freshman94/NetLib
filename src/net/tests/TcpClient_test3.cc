// TcpClient destructs in a different thread.

#include <log/Logger.h>
#include <net/EventLoopThread.h>
#include <net/TcpClient.h>

const int MicroSecsPerSec = 1000 * 1000;

void sleepUsec(int64_t usec) {
	struct timespec ts = { 0, 0 };
	ts.tv_sec = static_cast<time_t>(usec / MicroSecsPerSec);
	ts.tv_nsec = static_cast<long>(usec % MicroSecsPerSec * 1000);
	nanosleep(&ts, NULL);
}

int main(int argc, char* argv[])
{
	Logger::setLogLevel(Logger::DEBUG);

	EventLoopThread loopThread;
	{
		InetAddress serverAddr("127.0.0.1", 1234); // should succeed
		TcpClient client(loopThread.startLoop(), serverAddr, "TcpClient");
		client.connect();
		sleepUsec(500 * 1000);  // wait for connect
		client.disconnect();
	}

	sleepUsec(1000 * 1000);
}
