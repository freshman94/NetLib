#include "dispatcher_lite.h"

#include <protobuf/codec/query.pb.h>
#include <iostream>

using std::cout;
using std::endl;

void onUnknownMessageType(const TcpConnectionPtr&,
                          const MessagePtr& message)
{
  cout << "onUnknownMessageType: " << message->GetTypeName() << endl;
}

void onQuery(const TcpConnectionPtr&,
             const MessagePtr& message)
{
  cout << "onQuery: " << message->GetTypeName() << endl;
  std::shared_ptr<NetLib::Query> query = std::dynamic_pointer_cast<NetLib::Query>(message);
  assert(query != NULL);
}

void onAnswer(const TcpConnectionPtr&,
              const MessagePtr& message)
{
  cout << "onAnswer: " << message->GetTypeName() << endl;
  std::shared_ptr<NetLib::Answer> answer = std::dynamic_pointer_cast<NetLib::Answer>(message);
  assert(answer != NULL);
}

int main()
{
  GOOGLE_PROTOBUF_VERIFY_VERSION;

  ProtobufDispatcherLite dispatcher(onUnknownMessageType);
  dispatcher.registerMessageCallback(NetLib::Query::descriptor(), onQuery);
  dispatcher.registerMessageCallback(NetLib::Answer::descriptor(), onAnswer);

  TcpConnectionPtr conn;

  std::shared_ptr<NetLib::Query> query(new NetLib::Query);
  std::shared_ptr<NetLib::Answer> answer(new NetLib::Answer);
  std::shared_ptr<NetLib::Empty> empty(new NetLib::Empty);
  dispatcher.onProtobufMessage(conn, query);
  dispatcher.onProtobufMessage(conn, answer);
  dispatcher.onProtobufMessage(conn, empty);

  google::protobuf::ShutdownProtobufLibrary();
}

