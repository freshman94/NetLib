#include <log/AsyncLogging.h>
#include <log/LogFile.h>

#include <stdio.h>

AsyncLogging::AsyncLogging(const string& basename,
	off_t rollSize,
	int flushInterval)
	: flushInterval_(flushInterval),
	running_(false),
	basename_(basename),
	rollSize_(rollSize),
	thread_(std::bind(&AsyncLogging::threadFunc, this), "Logging"),
	latch_(1),
	mutex_(),
	cond_(mutex_),
	currentBuffer_(new Buffer),
	nextBuffer_(new Buffer),
	buffers_()
{
	currentBuffer_->bzero();
	nextBuffer_->bzero();
	buffers_.reserve(16);
}

void AsyncLogging::append(const char* logline, int len){	//前端：向日志线程发送日志消息
	MutexLockGuard lock(mutex_);
	//当前缓冲区空间足够
	if (currentBuffer_->avail() > len) {
		currentBuffer_->append(logline, len);
	}
	//当前缓冲区空间不足，push it, 寻找下一个可用的缓冲区
	else{
		buffers_.push_back(std::move(currentBuffer_));
		if (nextBuffer_){
			currentBuffer_ = std::move(nextBuffer_);//移动而非复制
		}
			
		else {
			//有大量的日志产生或后端写日志速度过慢，导致无可用缓冲时，新分配一块缓冲区
			currentBuffer_.reset(new Buffer);
		}	
		currentBuffer_->append(logline, len);
		cond_.notify();//缓冲区已满，通知后端开始写入日志数据
	}
}

void AsyncLogging::threadFunc()	//后端日志线程：收集消息并写入日志文件
{
	assert(running_ == true);
	latch_.countDown();
	LogFile output(basename_, rollSize_, false);
	BufferPtr newBuffer1(new Buffer);
	BufferPtr newBuffer2(new Buffer);
	newBuffer1->bzero();
	newBuffer2->bzero();
	BufferVector buffersToWrite;
	buffersToWrite.reserve(16);
	while (running_){
		assert(newBuffer1 && newBuffer1->length() == 0);
		assert(newBuffer2 && newBuffer2->length() == 0);
		assert(buffersToWrite.empty());

		{
			MutexLockGuard lock(mutex_);
			//等待条件触发：（1）buffer写满 （2）超时
			if (buffers_.empty())  // 没有用while
				cond_.waitForSeconds(flushInterval_);
			//条件满足，将当前缓冲移入buffers_
			buffers_.push_back(std::move(currentBuffer_));
			//move后，newBuffer1,newBuffer2均为空
			currentBuffer_ = std::move(newBuffer1);
			//将buffers_与buffersToWrite交换，这样后面的代码
			//可以在临界区之外安全地访问buffersToWrite,将其中日志写入文件
			buffersToWrite.swap(buffers_);
			if (!nextBuffer_)
				nextBuffer_ = std::move(newBuffer2);
		}

		assert(!buffersToWrite.empty());

		//日志消息堆积，直接丢弃多余的日志buffer
		if (buffersToWrite.size() > 25)
			buffersToWrite.erase(buffersToWrite.begin() + 2, buffersToWrite.end());

		//写入日志
		for (const auto& buffer : buffersToWrite)
			output.append(buffer->data(), buffer->length());

		//日志写完，丢弃多余的空间，只保留了2个Buffer
		if (buffersToWrite.size() > 2)
			buffersToWrite.resize(2);

		//重新填充newBuffer1、newBuffer2
		if (!newBuffer1){
			assert(!buffersToWrite.empty());
			newBuffer1 = std::move(buffersToWrite.back());
			buffersToWrite.pop_back();
			newBuffer1->reset();
		}

		if (!newBuffer2){
			assert(!buffersToWrite.empty());
			newBuffer2 = std::move(buffersToWrite.back());
			buffersToWrite.pop_back();
			newBuffer2->reset();
		}

		buffersToWrite.clear();
		output.flush();
	}
	output.flush();
}
