#pragma once

#include <base/noncopyable.h>
#include <net/TcpConnection.h>

#include <google/protobuf/message.h>
#include <map>
#include <memory>
#include <functional>

typedef std::shared_ptr<google::protobuf::Message> MessagePtr;

class ProtobufDispatcherLite : noncopyable
{
 public:
  typedef std::function<void (const TcpConnectionPtr&,
                                const MessagePtr&)> ProtobufMessageCallback;

  explicit ProtobufDispatcherLite(const ProtobufMessageCallback& defaultCb)
    : defaultCallback_(defaultCb){}

  void onProtobufMessage(const TcpConnectionPtr& conn,
                         const MessagePtr& message) const
  {
    CallbackMap::const_iterator it = callbacks_.find(message->GetDescriptor());
    if (it != callbacks_.end())
      it->second(conn, message);
    else
      defaultCallback_(conn, message);
  }

  void registerMessageCallback(const google::protobuf::Descriptor* desc,
                               const ProtobufMessageCallback& callback)
  {
    callbacks_[desc] = callback;
  }

 private:

  typedef std::map<const google::protobuf::Descriptor*, ProtobufMessageCallback> CallbackMap;
  CallbackMap callbacks_;
  ProtobufMessageCallback defaultCallback_;
};

