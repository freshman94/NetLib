#include <net/Buffer.h>
#include <net/SocketsOps.h>

#include <errno.h>
#include <sys/uio.h>

const char Buffer::CRLF[] = "\r\n";

const size_t Buffer::Prepend;
const size_t Buffer::InitialSize;

//readv结合栈上空间
ssize_t Buffer::readFd(int fd, int* savedErrno){
	char extrabuf[65536];
	struct iovec vec[2];
	const size_t writable = writableBytes();
	vec[0].iov_base = begin() + writeIndex_;
	vec[0].iov_len = writable;
	vec[1].iov_base = extrabuf;
	vec[1].iov_len = sizeof extrabuf;
	
	//缓冲区的可写空间大于extrabuf，则不使用extrabuf
	const int iovcnt = (writable < sizeof extrabuf) ? 2 : 1;
	const ssize_t n = sockets::readv(fd, vec, iovcnt);
	if (n < 0)
		*savedErrno = errno;
	else if (implicit_cast<size_t>(n) <= writable)
		writeIndex_ += n;
	else{
		writeIndex_ = buffer_.size();
		append(extrabuf, n - writable);
	}
	return n;
}

