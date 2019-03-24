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

void AsyncLogging::append(const char* logline, int len){	//ǰ�ˣ�����־�̷߳�����־��Ϣ
	MutexLockGuard lock(mutex_);
	//��ǰ�������ռ��㹻
	if (currentBuffer_->avail() > len) {
		currentBuffer_->append(logline, len);
	}
	//��ǰ�������ռ䲻�㣬push it, Ѱ����һ�����õĻ�����
	else{
		buffers_.push_back(std::move(currentBuffer_));
		if (nextBuffer_){
			currentBuffer_ = std::move(nextBuffer_);//�ƶ����Ǹ���
		}
			
		else {
			//�д�������־��������д��־�ٶȹ����������޿��û���ʱ���·���һ�黺����
			currentBuffer_.reset(new Buffer);
		}	
		currentBuffer_->append(logline, len);
		cond_.notify();//������������֪ͨ��˿�ʼд����־����
	}
}

void AsyncLogging::threadFunc()	//�����־�̣߳��ռ���Ϣ��д����־�ļ�
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
			//�ȴ�������������1��bufferд�� ��2����ʱ
			if (buffers_.empty())  // û����while
				cond_.waitForSeconds(flushInterval_);
			//�������㣬����ǰ��������buffers_
			buffers_.push_back(std::move(currentBuffer_));
			//move��newBuffer1,newBuffer2��Ϊ��
			currentBuffer_ = std::move(newBuffer1);
			//��buffers_��buffersToWrite��������������Ĵ���
			//�������ٽ���֮�ⰲȫ�ط���buffersToWrite,��������־д���ļ�
			buffersToWrite.swap(buffers_);
			if (!nextBuffer_)
				nextBuffer_ = std::move(newBuffer2);
		}

		assert(!buffersToWrite.empty());

		//��־��Ϣ�ѻ���ֱ�Ӷ����������־buffer
		if (buffersToWrite.size() > 25)
			buffersToWrite.erase(buffersToWrite.begin() + 2, buffersToWrite.end());

		//д����־
		for (const auto& buffer : buffersToWrite)
			output.append(buffer->data(), buffer->length());

		//��־д�꣬��������Ŀռ䣬ֻ������2��Buffer
		if (buffersToWrite.size() > 2)
			buffersToWrite.resize(2);

		//�������newBuffer1��newBuffer2
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
