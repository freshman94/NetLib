#include <log/LogFile.h>
#include <log/Logger.h>

#include <unistd.h>
#include <memory>

std::unique_ptr<LogFile> g_logFile;

void outputFunc(const char* msg, int len)
{
	g_logFile->append(msg, len);
}

void flushFunc()
{
	g_logFile->flush();
}

int main(int argc, char* argv[])
{
	string name = "logfile_test";
	g_logFile.reset(new LogFile(name, 200 * 1000));
	Logger::setOutput(outputFunc);
	Logger::setFlush(flushFunc);

	string line = "1234567890 abcdefghijklmnopqrstuvwxyz ABCDEFGHIJKLMNOPQRSTUVWXYZ ";

	for (int i = 0; i < 10000; ++i)
	{
		LOG_INFO << line << i;

		usleep(1000);
	}
}
