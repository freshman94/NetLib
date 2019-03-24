#pragma once
#include <base/noncopyable.h>
#include <base/StringPiece.h>
#include <base/Types.h>

#include <assert.h>
#include <string.h> // memcpy

//缓冲区大小
const int kSmallBuffer = 4000;
const int kLargeBuffer = 4000 * 1000;

//缓冲区
template<int SIZE>
class FixedBuffer : noncopyable{
public:
	FixedBuffer()
		: cur_(data_){}

	~FixedBuffer() {}

	void append(const char* buf, size_t len){
		if (implicit_cast<size_t>(avail()) > len){
			memcpy(cur_, buf, len);
			cur_ += len;
		}
	}

	const char* data() const { return data_; }
	int length() const { return static_cast<int>(cur_ - data_); }

	char* current() { return cur_; }
	int avail() const { return static_cast<int>(end() - cur_); }
	void add(size_t len) { cur_ += len; }

	void reset() { cur_ = data_; }
	void bzero() { memZero(data_, sizeof data_); }

	string toString() const { return string(data_, length()); }
	StringPiece toStringPiece() const { return StringPiece(data_, length()); }

private:
	const char* end() const { return data_ + sizeof data_; }
	char data_[SIZE];
	char* cur_;
};

//缓冲区的输出流

class LogStream : noncopyable{
public:
	typedef FixedBuffer<kSmallBuffer> Buffer;	//小缓冲区

	LogStream& operator<<(bool v){
		buffer_.append(v ? "1" : "0", 1);
		return *this;
	}

	LogStream& operator<<(short);
	LogStream& operator<<(unsigned short);
	LogStream& operator<<(int);
	LogStream& operator<<(unsigned int);
	LogStream& operator<<(long);
	LogStream& operator<<(unsigned long);
	LogStream& operator<<(long long);
	LogStream& operator<<(unsigned long long);

	LogStream& operator<<(const void*);

	LogStream& operator<<(float v){
		*this << static_cast<double>(v);
		return *this;
	}
	LogStream& operator<<(double);
	LogStream& operator<<(long double);

	LogStream& operator<<(char v){
		buffer_.append(&v, 1);
		return *this;
	}

	LogStream& operator<<(const char* str){
		if (str)
			buffer_.append(str, strlen(str));
		else
			buffer_.append("(null)", 6);
		return *this;
	}

	LogStream& operator<<(const unsigned char* str){
		return operator<<(reinterpret_cast<const char*>(str));
	}

	LogStream& operator<<(const string& v){
		buffer_.append(v.c_str(), v.size());
		return *this;
	}

	LogStream& operator<<(const StringPiece& v){
		buffer_.append(v.data(), v.size());
		return *this;
	}

	LogStream& operator<<(const Buffer& v){
		*this << v.toStringPiece();
		return *this;
	}

	void append(const char* data, int len) { buffer_.append(data, len); }
	const Buffer& buffer() const { return buffer_; }
	void resetBuffer() { buffer_.reset(); }

private:
	void staticCheck();

	template<typename T>
	void formatInteger(T);

	Buffer buffer_;

	//整型数据的最大位数（10进制表示的）
	static const int kMaxNumericSize = 32;
};
