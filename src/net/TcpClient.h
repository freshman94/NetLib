#pragma once

#include <base/Mutex.h>
#include <net/TcpConnection.h>

using std::placeholders::_1;
class Connector;
typedef std::shared_ptr<Connector> ConnectorPtr;

class TcpClient : noncopyable{
 public:
  TcpClient(EventLoop* loop, const InetAddress& serverAddr,
            const string& nameArg);
  ~TcpClient();

  void connect();
  void disconnect();
  void stop();

  TcpConnectionPtr connection() const
  {
    MutexLockGuard lock(mutex_);
    return connection_;
  }

  EventLoop* getLoop() const { return loop_; }
  bool retry() const { return retry_; }
  void enableRetry() { retry_ = true; }
  const string& name() const{ return name_; }

  void setConnectionCallback(const Callback& cb) 
  { connectionCallback_ = std::move(cb); }

  void setMessageCallback(MessageCallback cb)
  { messageCallback_ = std::move(cb); }


 private:
  void newConnection(int sockfd);
  void removeConnection(const TcpConnectionPtr& conn);

  EventLoop* loop_;
  ConnectorPtr connector_;
  const string name_;
  Callback connectionCallback_;
  MessageCallback messageCallback_;
  bool retry_;
  bool connect_;
  int nextConnId_;
  mutable MutexLock mutex_;
  TcpConnectionPtr connection_;
};
