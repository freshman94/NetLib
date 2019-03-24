#pragma once

#include <base/copyable.h>
#include <base/StringPiece.h>
#include <base/Types.h>

#include <algorithm>
#include <vector>
#include <assert.h>
#include <string.h>


/// +-------------------+------------------+------------------+
/// | prependable bytes |  readable bytes  |  writable bytes  |
/// |                   |     (CONTENT)    |                  |
/// +-------------------+------------------+------------------+
/// |                   |                  |                  |
/// 0      <=      readerIndex   <=   writerIndex    <=     size


class Buffer : public copyable{
public:
	static const size_t Prepend = 8;
	static const size_t InitialSize = 1024;

	explicit Buffer(size_t initialSize = InitialSize)
		: buffer_(Prepend + initialSize),
		readIndex_(Prepend),
		writeIndex_(Prepend)
	{
		assert(readableBytes() == 0);
		assert(writableBytes() == initialSize);
		assert(prependableBytes() == Prepend);
	}

	void swap(Buffer& rhs){
		buffer_.swap(rhs.buffer_);
		std::swap(readIndex_, rhs.readIndex_);
		std::swap(writeIndex_, rhs.writeIndex_);
	}

	size_t readableBytes() const{ return writeIndex_ - readIndex_;}
	size_t writableBytes() const{ return buffer_.size() - writeIndex_;}
	size_t prependableBytes() const{ return readIndex_;}

	void ensureWritableBytes(size_t len){
		if (writableBytes() < len)
			makeSpace(len);
		assert(writableBytes() >= len);
	}
	const char* readablePtr() const { return begin() + readIndex_; }
	char* writablePtr(){ return begin() + writeIndex_;}
	const char* writablePtr() const{ return begin() + writeIndex_;}

	void Written(size_t len){
		assert(len <= writableBytes());
		writeIndex_ += len;
	}

	void unwrite(size_t len){
		assert(len <= readableBytes());
		writeIndex_ -= len;
	}

	const char* findCRLF() const{
		const char* crlf = std::search(readablePtr(), writablePtr(),CRLF,CRLF + 2);
		return crlf == writablePtr() ? NULL : crlf;
	}

	const char* findCRLF(const char* start) const{
		assert(readablePtr() <= start);
		assert(start <= writablePtr());
		const char* crlf = std::search(start, writablePtr(),CRLF,CRLF + 2);
		return crlf == writablePtr() ? NULL : crlf;
	}

	const char* findEOL() const{
		const void* eol = memchr(readablePtr(), '\n', readableBytes());
		return static_cast<const char*>(eol);
	}

	const char* findEOL(const char* start) const{
		assert(readablePtr() <= start);
		assert(start <= writablePtr());
		const void* eol = memchr(start, '\n', writablePtr() - start);
		return static_cast<const char*>(eol);
	}

	//读数据
	void retrieve(size_t len){
		assert(len <= readableBytes());
		if (len < readableBytes())
			readIndex_ += len;
		else
			retrieveAll();
	}

	//全部数据读完了，恢复index以备新一轮使用
	void retrieveAll(){
		readIndex_ = Prepend;
		writeIndex_ = Prepend;
	}

	string retrieveAllAsString(){ return retrieveAsString(readableBytes());}

	string retrieveAsString(size_t len){
		assert(len <= readableBytes());
		string result(readablePtr(), len);
		retrieve(len);
		return result;
	}

	void retrieveUntil(const char* end){
		assert(readablePtr() <= end);
		assert(end <= writablePtr());
		retrieve(end - readablePtr());
	}

	//写数据
	void append(const StringPiece& str){ append(str.data(), str.size());}
	void append(const void* data, size_t len) { append(static_cast<const char*>(data), len); }

	void append(const char* data, size_t len){
		ensureWritableBytes(len);
		std::copy(data, data + len, writablePtr());
		Written(len);
	}

	//添加数据到预留区
	void prepend(const void* data, size_t len){
		assert(len <= prependableBytes());
		readIndex_ -= len;
		const char* d = static_cast<const char*>(data);
		std::copy(d, d + len, begin() + readIndex_);
	}

	void shrink(size_t reserve){
		Buffer other;
		other.ensureWritableBytes(readableBytes() + reserve);
		other.append(readablePtr(), readableBytes());
		swap(other);
	}

	size_t Capacity() const{ return buffer_.capacity();}

	//将数据读入缓冲区
	ssize_t readFd(int fd, int* savedErrno);

private:
	char* begin(){ return &*buffer_.begin();}
	const char* begin() const{ return &*buffer_.begin();}

	void makeSpace(size_t len){
		if (writableBytes() + prependableBytes() < len + Prepend)
			buffer_.resize(writeIndex_ + len);
		else{
			//自动腾挪：将可读区的数据移到前面，以腾出空间
			assert(Prepend < readIndex_);
			size_t readable = readableBytes();
			std::copy(begin() + readIndex_,
				begin() + writeIndex_,
				begin() + Prepend);
			readIndex_ = Prepend;
			writeIndex_ = readIndex_ + readable;
			assert(readable == readableBytes());
		}
	}

private:
	std::vector<char> buffer_;
	size_t readIndex_;
	size_t writeIndex_;
	static const char CRLF[];
};
