#include "dispatcher.h"
#include "codec.h"
#include <protobuf/codec/query.pb.h>

#include <log/Logger.h>
#include <base/Mutex.h>
#include <net/EventLoop.h>
#include <net/TcpClient.h>

#include <stdio.h>
#include <unistd.h>


typedef std::shared_ptr<NetLib::Empty> EmptyPtr;
typedef std::shared_ptr<NetLib::Answer> AnswerPtr;

google::protobuf::Message* messageToSend;

class QueryClient : noncopyable
{
public:
	QueryClient(EventLoop* loop,
		const InetAddress& serverAddr)
		: loop_(loop),
		client_(loop, serverAddr, "QueryClient"),
		dispatcher_(std::bind(&QueryClient::onUnknownMessage, this, _1, _2)),
		codec_(std::bind(&ProtobufDispatcher::onProtobufMessage, &dispatcher_, _1, _2))
	{
		dispatcher_.registerMessageCallback<NetLib::Answer>(
			std::bind(&QueryClient::onAnswer, this, _1, _2));
		dispatcher_.registerMessageCallback<NetLib::Empty>(
			std::bind(&QueryClient::onEmpty, this, _1, _2));
		client_.setConnectionCallback(
			std::bind(&QueryClient::onConnection, this, _1));
		client_.setMessageCallback(
			std::bind(&ProtobufCodec::onMessage, &codec_, _1, _2));
	}

	void connect()
	{
		client_.connect();
	}

private:

	void onConnection(const TcpConnectionPtr& conn)
	{
		LOG_INFO << conn->localAddress().toIpPort() << " -> "
			<< conn->peerAddress().toIpPort() << " is "
			<< (conn->connected() ? "UP" : "DOWN");

		if (conn->connected())
			codec_.send(conn, *messageToSend);
		else
			loop_->quit();
	}

	void onUnknownMessage(const TcpConnectionPtr&,
		const MessagePtr& message)
	{
		LOG_INFO << "onUnknownMessage: " << message->GetTypeName();
	}

	void onAnswer(const TcpConnectionPtr&,
		const AnswerPtr& message)
	{
		LOG_INFO << "onAnswer:\n" << message->GetTypeName() << message->DebugString();
	}

	void onEmpty(const TcpConnectionPtr&,
		const EmptyPtr& message)
	{
		LOG_INFO << "onEmpty: " << message->GetTypeName();
	}

	EventLoop* loop_;
	TcpClient client_;
	ProtobufDispatcher dispatcher_;
	ProtobufCodec codec_;
};

int main(int argc, char* argv[])
{
	LOG_INFO << "pid = " << getpid();
	if (argc > 2)
	{
		EventLoop loop;
		uint16_t port = static_cast<uint16_t>(atoi(argv[2]));
		InetAddress serverAddr(argv[1], port);

		NetLib::Query query;
		query.set_id(1);
		query.set_questioner("DJH");
		query.add_question("Running?");
		NetLib::Empty empty;
		messageToSend = &query;

		if (argc > 3 && argv[3][0] == 'e')
		{
			messageToSend = &empty;
		}

		QueryClient client(&loop, serverAddr);
		client.connect();
		loop.loop();
	}
	else
	{
		printf("Usage: %s host_ip port [q|e]\n", argv[0]);
	}
}

