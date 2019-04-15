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

	void retrieveInt64()
	{
		retrieve(sizeof(int64_t));
	}

	void retrieveInt32()
	{
		retrieve(sizeof(int32_t));
	}

	void retrieveInt16()
	{
		retrieve(sizeof(int16_t));
	}

	void retrieveInt8()
	{
		retrieve(sizeof(int8_t));
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


	void appendInt64(int64_t x)
	{
		int64_t be64 = htobe64(x);
		append(&be64, sizeof be64);
	}

	void appendInt32(int32_t x)
	{
		int32_t be32 = htobe32(x);
		append(&be32, sizeof be32);
	}

	void appendInt16(int16_t x)
	{
		int16_t be16 = htobe16(x);
		append(&be16, sizeof be16);
	}

	void appendInt8(int8_t x)
	{
		append(&x, sizeof x);
	}


	int64_t readInt64()
	{
		int64_t result = readableBytesInt64();
		retrieveInt64();
		return result;
	}

	int32_t readInt32()
	{
		int32_t result = readableBytesInt32();
		retrieveInt32();
		return result;
	}

	int16_t readInt16()
	{
		int16_t result = readableBytesInt16();
		retrieveInt16();
		return result;
	}

	int8_t readInt8()
	{
		int8_t result = readableBytesInt8();
		retrieveInt8();
		return result;
	}


	int64_t readableBytesInt64() const
	{
		assert(readableBytes() >= sizeof(int64_t));
		int64_t be64 = 0;
		memcpy(&be64, readablePtr(), sizeof be64);
		return be64toh(be64);
	}

	int32_t readableBytesInt32() const
	{
		assert(readableBytes() >= sizeof(int32_t));
		int32_t be32 = 0;
		memcpy(&be32, readablePtr(), sizeof be32);
		return be32toh(be32);
	}

	int16_t readableBytesInt16() const
	{
		assert(readableBytes() >= sizeof(int16_t));
		int16_t be16 = 0;
		::memcpy(&be16, readablePtr(), sizeof be16);
		return be16toh(be16);
	}

	int8_t readableBytesInt8() const
	{
		assert(readableBytes() >= sizeof(int8_t));
		int8_t x = *readablePtr();
		return x;
	}



	//添加数据到预留区
	void prepend(const void* data, size_t len){
		assert(len <= prependableBytes());
		readIndex_ -= len;
		const char* d = static_cast<const char*>(data);
		std::copy(d, d + len, begin() + readIndex_);
	}

	void prependInt64(int64_t x)
	{
		int64_t be64 = htobe64(x);
		prepend(&be64, sizeof be64);
	}

	///
	/// Prepend int32_t using network endian
	///
	void prependInt32(int32_t x)
	{
		int32_t be32 = htobe32(x);
		prepend(&be32, sizeof be32);
	}

	void prependInt16(int16_t x)
	{
		int16_t be16 = htobe16(x);
		prepend(&be16, sizeof be16);
	}

	void prependInt8(int8_t x)
	{
		prepend(&x, sizeof x);
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
