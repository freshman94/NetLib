#pragma once

#include <base/noncopyable.h>
#include <net/TcpConnection.h>

#include <google/protobuf/message.h>
#include <map>
#include <memory>
#include <type_traits>

typedef std::shared_ptr<google::protobuf::Message> MessagePtr;

class CallBack : noncopyable
{
 public:
  virtual ~CallBack() = default;
  virtual void onMessage(const TcpConnectionPtr&,
                         const MessagePtr& message) const = 0;
};

template <typename T>
class CallBackT : public CallBack
{
  static_assert(std::is_base_of<google::protobuf::Message, T>::value,
                "T must be derived from gpb::Message.");
 public:
  typedef std::function<void (const TcpConnectionPtr&,
          const std::shared_ptr<T>& message)> ProtobufMessageTCallback;

  CallBackT(const ProtobufMessageTCallback& callback)
    : callback_(callback) {}

  void onMessage(const TcpConnectionPtr& conn,
                 const MessagePtr& message) const override
  {
    std::shared_ptr<T> concrete = std::dynamic_pointer_cast<T>(message);
    assert(concrete != NULL);
    callback_(conn, concrete);
  }

 private:
  ProtobufMessageTCallback callback_;
};

class ProtobufDispatcher
{
 public:
  typedef std::function<void (const TcpConnectionPtr&,
          const MessagePtr& message)> ProtobufMessageCallback;

  explicit ProtobufDispatcher(const ProtobufMessageCallback& defaultCb)
    : defaultCallback_(defaultCb) {}

  void onProtobufMessage(const TcpConnectionPtr& conn,
                         const MessagePtr& message) const
  {
    CallbackMap::const_iterator it = callbacks_.find(message->GetDescriptor());
    if (it != callbacks_.end())
      it->second->onMessage(conn, message);
    else
      defaultCallback_(conn, message);
  }

  template<typename T>
  void registerMessageCallback(const typename CallBackT<T>::ProtobufMessageTCallback& callback)
  {
    std::shared_ptr<CallBackT<T> > pd(new CallBackT<T>(callback));
    callbacks_[T::descriptor()] = pd;
  }

 private:
  typedef std::map<const google::protobuf::Descriptor*, std::shared_ptr<CallBack> > CallbackMap;

  CallbackMap callbacks_;
  ProtobufMessageCallback defaultCallback_;
};

