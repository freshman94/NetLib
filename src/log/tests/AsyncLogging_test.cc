#include <log/AsyncLogging.h>
#include <log/Logger.h>
#include <base/Thread.h>
#include <base/CountDownLatch.h>

#include <stdio.h>
#include <sys/resource.h>
#include <unistd.h>
#include <string>

off_t RollSize = 500 * 1000 * 1000;

AsyncLogging* g_asyncLog = NULL;

void asyncOutput(const char* msg, int len)
{
	g_asyncLog->append(msg, len);
}

void bench(bool longLog)
{
	Logger::setOutput(asyncOutput);

	int cnt = 0;
	const int Batch = 1000;
	std::string empty = " ";
	std::string longStr(3000, 'X');
	longStr += " ";

	for (int t = 0; t < 30; ++t)
	{
		time_t start = time(NULL);
		for (int i = 0; i < Batch; ++i)
		{
			LOG_INFO << "Hello 0123456789" << " abcdefghijklmnopqrstuvwxyz "
				<< (longLog ? longStr : empty)
				<< cnt;
			++cnt;
		}
		time_t end = time(NULL);
		printf("s/logMessage£º %f\n", (end - start)/ Batch);
		struct timespec ts = { 0, 500 * 1000 * 1000 };
		nanosleep(&ts, NULL);
	}
}

int main(int argc, char* argv[])
{
	{
		// set max virtual memory to 2GB.
		size_t OneGB = 1000 * 1024 * 1024;
		rlimit rl = { 2 * OneGB, 2 * OneGB };
		setrlimit(RLIMIT_AS, &rl);
	}

	printf("pid = %d\n", getpid());

	char name[256] = { 0 };
	strncpy(name, argv[0], sizeof name - 1);
	AsyncLogging log(::basename(name), RollSize);
	log.start();
	g_asyncLog = &log;

	bool longLog = argc > 1;
	bench(longLog);
}
