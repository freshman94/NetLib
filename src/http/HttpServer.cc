#include <http/HttpServer.h>
#include <http/HttpParser.h>
#include <http/HttpRequest.h>
#include <http/HttpResponse.h>
#include <log/Logger.h>

void defaultHttpCallback(const HttpRequest&, HttpResponse* resp) {
	resp->setStatusCode(HttpResponse::NotFound404);
	resp->setStatusMessage("Not Found!");
	resp->setCloseConnection(true);
}


HttpServer::HttpServer(EventLoop* loop, const InetAddress& listenAddr,
	const string& name, int numThreads)
	: server_(loop, listenAddr, name, numThreads),
	httpCallback_(defaultHttpCallback)
{
	server_.setConnectionCallback(
		std::bind(&HttpServer::onConnection, this, _1));
	server_.setMessageCallback(
		std::bind(&HttpServer::onMessage, this, _1, _2));
}

void HttpServer::start() {
	LOG_WARN << "HttpServer[" << server_.name()
		<< "] starts listenning on " << server_.ipPort();
	server_.start();
}

void HttpServer::onConnection(const TcpConnectionPtr& conn) {
	if (conn->connected())
		conn->setContext(HttpParser());
}

void HttpServer::onMessage(const TcpConnectionPtr& conn, Buffer* buf) {
	HttpParser* parser = boost::any_cast<HttpParser>(conn->getMutableContext());

	if (!parser->parseRequest(buf)){
		conn->send("HTTP/1.1 400 Bad Request\r\n\r\n");
		conn->shutdown();
	}

	if (parser->gotAll()){
		onRequest(conn, parser->request());
		parser->reset();
	}
}

void HttpServer::onRequest(const TcpConnectionPtr& conn, const HttpRequest& req){
	const string& connection = req.getHeader("Connection");
	bool close = connection == "close" ||
		(req.getVersion() == HttpRequest::Http10 && connection != "Keep-Alive");
	HttpResponse response(close);
	httpCallback_(req, &response);
	Buffer buf;
	response.appendToBuffer(&buf);
	conn->send(&buf);
	if (response.closeConnection())
		conn->shutdown();
}

