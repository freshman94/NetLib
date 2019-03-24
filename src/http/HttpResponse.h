#pragma once

#include <base/copyable.h>
#include <base/Types.h>

#include <map>

class Buffer;

class HttpResponse : public copyable{
public:
	enum HttpStatusCode{
		Unknown,
		OK200 = 200,
		MovedPermanently301 = 301,
		BadRequest400 = 400,
		NotFound404 = 404,
	};

	explicit HttpResponse(bool close)
		: statusCode_(Unknown), closeConnection_(close) {}

	void setStatusCode(HttpStatusCode code){ statusCode_ = code;}
	void setStatusMessage(const string& message){ statusMessage_ = message;}
	void setCloseConnection(bool on){ closeConnection_ = on;}
	bool closeConnection() const{ return closeConnection_;}
	void setContentType(const string& contentType){ addHeader("Content-Type", contentType);}
	void setBody(const string& body) { body_ = body; }

	void addHeader(const string& key, const string& value){
		headers_[key] = value;
	}

	void appendToBuffer(Buffer* output) const;

private:
	std::map<string, string> headers_;
	HttpStatusCode statusCode_;
	string statusMessage_;
	bool closeConnection_;
	string body_;
};
