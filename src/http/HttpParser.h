#pragma once

#include <base/copyable.h>
#include <http/HttpRequest.h>

class Buffer;

class HttpParser : public copyable{
 public:
  enum ParseState{	//������״̬
    ParseRequestLine,
    ParseHeaders,
    ParseBody,
    ParseGotAll,
  };

  enum ParseRequestLineState{	//���������е�״̬
	  ParseMethod,
	  ParseURL,
	  ParseVersion,
	  ParseFinished,
  };

  HttpParser()
	: state_(ParseRequestLine), 
	  requestLineState_(ParseMethod),
	  bodyLength_(0)
  {}

  bool parseRequest(Buffer* buf);

  bool gotAll() const { return state_ == ParseGotAll; }
  const HttpRequest& request() const { return request_; }
  HttpRequest& request() { return request_; }

  void reset(){
    state_ = ParseRequestLine;
	requestLineState_ = ParseMethod;
    HttpRequest dummy;
    request_.swap(dummy);
  }

 private:
  bool parseRequestLine(const char* begin, const char* end);

  ParseState state_;
  ParseRequestLineState requestLineState_;
  HttpRequest request_;
  size_t bodyLength_;
};
