#include "dispatcher.h"
#include <protobuf/codec/query.pb.h>

#include <iostream>

using std::cout;
using std::endl;

typedef std::shared_ptr<NetLib::Query> QueryPtr;
typedef std::shared_ptr<NetLib::Answer> AnswerPtr;

void test_down_pointer_cast()
{
  ::std::shared_ptr<google::protobuf::Message> msg(new NetLib::Query);
  ::std::shared_ptr<NetLib::Query> query(std::dynamic_pointer_cast<NetLib::Query>(msg));
  assert(msg && query);
}

void onQuery(const TcpConnectionPtr&,
             const QueryPtr& message)
{
  cout << "onQuery: " << message->GetTypeName() << endl;
}

void onAnswer(const TcpConnectionPtr&,
              const AnswerPtr& message)
{
  cout << "onAnswer: " << message->GetTypeName() << endl;
}

void onUnknownMessageType(const TcpConnectionPtr&,
                          const MessagePtr& message)
{
  cout << "onUnknownMessageType: " << message->GetTypeName() << endl;
}

int main()
{
  GOOGLE_PROTOBUF_VERIFY_VERSION;
  test_down_pointer_cast();

  ProtobufDispatcher dispatcher(onUnknownMessageType);
  dispatcher.registerMessageCallback<NetLib::Query>(onQuery);
  dispatcher.registerMessageCallback<NetLib::Answer>(onAnswer);

  TcpConnectionPtr conn;

  std::shared_ptr<NetLib::Query> query(new NetLib::Query);
  std::shared_ptr<NetLib::Answer> answer(new NetLib::Answer);
  std::shared_ptr<NetLib::Empty> empty(new NetLib::Empty);
  dispatcher.onProtobufMessage(conn, query);
  dispatcher.onProtobufMessage(conn, answer);
  dispatcher.onProtobufMessage(conn, empty);

  google::protobuf::ShutdownProtobufLibrary();
}

