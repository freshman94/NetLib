#include <log/Logger.h>

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sstream>

Logger::LogLevel initLogLevel()
{
	if (getenv("HttpServer_LOG_TRACE"))
		return Logger::TRACE;
	else if (getenv("HttpServer_LOG_DEBUG"))
		return Logger::DEBUG;
	else
		return Logger::INFO;
}

Logger::LogLevel g_logLevel = initLogLevel();

const char* LogLevelName[Logger::NUM_LOG_LEVELS] =
{
	"TRACE ",
	"DEBUG ",
	"INFO  ",
	"WARN  ",
	"ERROR ",
	"FATAL ",
};

inline LogStream& operator<<(LogStream& s, const Logger::SourceFile& v)
{
  s.append(v.data_, v.size_);
  return s;
}

//Ĭ�������stdout
void defaultOutput(const char* msg, int len){
	size_t n = fwrite(msg, 1, len, stdout);
	(void)n;
}

void defaultFlush(){
	fflush(stdout);
}

Logger::OutputFunc g_output = defaultOutput;
Logger::FlushFunc g_flush = defaultFlush;

Logger::Impl::Impl(LogLevel level, const SourceFile& file, int line)
	: stream_(),
	level_(level),
	line_(line),
	basename_(file)
{
	//һ����־����ʼ
	formatTime();
	stream_ << LogLevelName[level];
}

void Logger::Impl::formatTime() {
	char time_str[26];
	struct tm tm;
	time_t now;
	now = time(NULL);
	localtime_r(&now, &tm);
	strftime(time_str, sizeof time_str, "%Y-%m-%d %H:%M:%S ", &tm);
	stream_ << time_str;
}

//һ����־�Ľ���
void Logger::Impl::finish(){
	stream_ << " - " << basename_ << ':' << line_ << '\n';
}

Logger::Logger(SourceFile file, int line)
	: impl_(INFO, file, line)
{}

Logger::Logger(SourceFile file, int line, LogLevel level, const char* func)
	: impl_(level, file, line)
{
	impl_.stream_ << func << ' ';
}

Logger::Logger(SourceFile file, int line, LogLevel level)
	: impl_(level, file, line)
{}

//��������ӡ������Ϣ��������־
//����־����ΪFATAL,��ջ��棬abort
Logger::~Logger(){
	impl_.finish();
	const LogStream::Buffer& buf(stream().buffer());
	g_output(buf.data(), buf.length());
	if (impl_.level_ == FATAL){
		g_flush();
		abort();
	}
}

void Logger::setLogLevel(Logger::LogLevel level){
	g_logLevel = level;
}

void Logger::setOutput(OutputFunc out){
	g_output = out;
}

void Logger::setFlush(FlushFunc flush){
	g_flush = flush;
}
