#include "codec.h"
#include "dispatcher.h"
#include <protobuf/codec/query.pb.h>

#include <log/Logger.h>
#include <base/Mutex.h>
#include <net/EventLoop.h>
#include <net/TcpServer.h>

#include <stdio.h>
#include <unistd.h>

typedef std::shared_ptr<NetLib::Query> QueryPtr;
typedef std::shared_ptr<NetLib::Answer> AnswerPtr;

class QueryServer : noncopyable
{
public:
	QueryServer(EventLoop* loop,
		const InetAddress& listenAddr)
		: server_(loop, listenAddr, "QueryServer"),
		dispatcher_(std::bind(&QueryServer::onUnknownMessage, this, _1, _2)),
		codec_(std::bind(&ProtobufDispatcher::onProtobufMessage, &dispatcher_, _1, _2))
	{
		dispatcher_.registerMessageCallback<NetLib::Query>(
			std::bind(&QueryServer::onQuery, this, _1, _2));
		dispatcher_.registerMessageCallback<NetLib::Answer>(
			std::bind(&QueryServer::onAnswer, this, _1, _2));
		server_.setConnectionCallback(
			std::bind(&QueryServer::onConnection, this, _1));
		server_.setMessageCallback(
			std::bind(&ProtobufCodec::onMessage, &codec_, _1, _2));
	}

	void start()
	{
		server_.start();
	}

private:
	void onConnection(const TcpConnectionPtr& conn)
	{
		LOG_INFO << conn->localAddress().toIpPort() << " -> "
			<< conn->peerAddress().toIpPort() << " is "
			<< (conn->connected() ? "UP" : "DOWN");
	}

	void onUnknownMessage(const TcpConnectionPtr& conn,
		const MessagePtr& message)
	{
		LOG_INFO << "onUnknownMessage: " << message->GetTypeName();
		conn->shutdown();
	}

	void onQuery(const TcpConnectionPtr& conn,
		const QueryPtr& message)
	{
		LOG_INFO << "onQuery:\n" << message->GetTypeName() << message->DebugString();
		NetLib::Answer answer;
		answer.set_id(1);
		answer.set_questioner("DJH");
		answer.set_answerer("blog.csdn.net/Solstice");
		answer.add_solution("Jump!");
		answer.add_solution("Win!");
		codec_.send(conn, answer);

		conn->shutdown();
	}

	void onAnswer(const TcpConnectionPtr& conn,
		const AnswerPtr& message)
	{
		LOG_INFO << "onAnswer: " << message->GetTypeName();
		conn->shutdown();
	}

	TcpServer server_;
	ProtobufDispatcher dispatcher_;
	ProtobufCodec codec_;
};

int main(int argc, char* argv[])
{
	LOG_INFO << "pid = " << getpid();
	if (argc > 1)
	{
		EventLoop loop;
		uint16_t port = static_cast<uint16_t>(atoi(argv[1]));
		InetAddress serverAddr(port);
		QueryServer server(&loop, serverAddr);
		server.start();
		loop.loop();
	}
	else
	{
		printf("Usage: %s port\n", argv[0]);
	}
}

