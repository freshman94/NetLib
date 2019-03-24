#pragma once

#include <log/LogStream.h>
#include <base/Types.h>

class Logger{
public:
	enum LogLevel
	{
		TRACE,
		DEBUG,
		INFO,
		WARN,
		ERROR,
		FATAL,
		NUM_LOG_LEVELS,	//6种日志级别
	};
	//用于在编译器得到源文件的basename
	class SourceFile{
	public:
		template<int N>
		SourceFile(const char(&arr)[N])
			: data_(arr),
			size_(N - 1)	//去除'\0'的size
		{
			const char* slash = strrchr(data_, '/'); // builtin function
			if (slash){
				data_ = slash + 1;
				size_ -= static_cast<int>(data_ - arr);
			}
		}

		explicit SourceFile(const char* filename)
			: data_(filename)
		{
			const char* slash = strrchr(filename, '/');
			if (slash)
				data_ = slash + 1;
			size_ = static_cast<int>(strlen(data_));
		}

		const char* data_;
		int size_;
	};

	Logger(SourceFile file, int line);
	Logger(SourceFile file, int line, LogLevel level);
	Logger(SourceFile file, int line, LogLevel level, const char* func);
	~Logger();

	LogStream& stream() { return impl_.stream_; }

	static LogLevel logLevel();
	static void setLogLevel(LogLevel level);

	typedef void(*OutputFunc)(const char* msg, int len);
	typedef void(*FlushFunc)();
	static void setOutput(OutputFunc);
	static void setFlush(FlushFunc);

private:
	class Impl{
	public:
		typedef Logger::LogLevel LogLevel;
		Impl(LogLevel level, const SourceFile& file, int line);
		void formatTime();
		void finish();

		LogStream stream_;
		LogLevel level_;
		int line_;
		SourceFile basename_;
	};

	Impl impl_;
};

extern Logger::LogLevel g_logLevel;

inline Logger::LogLevel Logger::logLevel(){ return g_logLevel;}
//正常的输出: 创建了Logger的临时变量，每次调用完即会调用Logger的析构函数
#define LOG_TRACE if (Logger::logLevel() <= Logger::TRACE) \
  Logger(__FILE__, __LINE__, Logger::TRACE, __func__).stream()
#define LOG_DEBUG if (Logger::logLevel() <= Logger::DEBUG) \
  Logger(__FILE__, __LINE__, Logger::DEBUG, __func__).stream()
#define LOG_INFO if (Logger::logLevel() <= Logger::INFO) \
  Logger(__FILE__, __LINE__).stream()

//警告级别以上的输出
#define LOG_WARN Logger(__FILE__, __LINE__, Logger::WARN).stream()
#define LOG_ERROR Logger(__FILE__, __LINE__, Logger::ERROR).stream()
#define LOG_FATAL Logger(__FILE__, __LINE__, Logger::FATAL).stream()

