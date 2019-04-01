#pragma once
#include <net/TcpServer.h>

class HttpRequest;
class HttpResponse;

class HttpServer : noncopyable{
 public:
  typedef std::function<void(const HttpRequest&, HttpResponse*)> HttpCallback;

  HttpServer(EventLoop* loop, const InetAddress& listenAddr,
	  const string& nameArg = string(), int numThreads = 0);

  EventLoop* getLoop() const { return server_.getLoop(); }
  void setHttpCallback(const HttpCallback& cb){ httpCallback_ = cb;}
  void start();

 private:
  void onConnection(const TcpConnectionPtr& conn);
  void onMessage(const TcpConnectionPtr& conn, Buffer* buf);
  void onRequest(const TcpConnectionPtr&, const HttpRequest&);

  TcpServer server_;
  HttpCallback httpCallback_;
};
