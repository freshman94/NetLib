#pragma once

#include <base/copyable.h>
#include <base/Types.h>

#include <unordered_map>
#include <assert.h>
#include <stdio.h>

class HttpRequest : public copyable{
public:
	enum Method{ Invalid, Get, Post, Head, Put, Delete};
	enum Version{ Unknown, Http10, Http11};

	HttpRequest(): method_(Invalid),version_(Unknown){}

	Method method() const { return method_; }
	void setVersion(Version v){ version_ = v;}
	Version getVersion() const{ return version_;}
	const string& path() const { return path_; }
	const std::unordered_map<string, string>& headers() const { return headers_; }

	bool setMethod(const char* start, const char* end){
		assert(method_ == Invalid);
		string m(start, end);
		if (m == "GET")
			method_ = Get;
		else if (m == "POST")
			method_ = Post;
		else if (m == "HEAD")
			method_ = Head;
		else if (m == "PUT")
			method_ = Put;
		else if (m == "DELETE")
			method_ = Delete;
		else
			method_ = Invalid;
		return method_ != Invalid;
	}

	const char* methodString() const{
		const char* result = "UNKNOWN";
		switch (method_)
		{
		case Get:
			result = "GET";
			break;
		case Post:
			result = "POST";
			break;
		case Head:
			result = "HEAD";
			break;
		case Put:
			result = "PUT";
			break;
		case Delete:
			result = "DELETE";
			break;
		default:
			break;
		}
		return result;
	}

	void setPath(const char* start, const char* end){
		path_.assign(start, end);
	}

	void addParas(const char* start, const char* equal, const char* end){
		std::string field(start, equal++);
		std::string value(equal, end);
		paras_[field] = value;
	}

	const std::string getParas(const std::string& field) const{
		auto ite = paras_.find(field);
		if (ite != paras_.end())
			return ite->second;
		return "";
	}
	
	void addHeader(const char* start, const char* colon, const char* end){
		string field(start, colon);
		++colon;
		while (colon < end && isspace(*colon))
			++colon;
		string value(colon, end);
		while (!value.empty() && isspace(value[value.size() - 1]))
			value.resize(value.size() - 1);
		headers_[field] = value;
	}

	string getHeader(const string& field) const{
		string result;
		std::unordered_map<string, string>::const_iterator ite = headers_.find(field);
		if (ite != headers_.end())
			result = ite->second;
		return result;
	}

	void swap(HttpRequest& rhs){
		std::swap(method_, rhs.method_);
		std::swap(version_, rhs.version_);
		path_.swap(rhs.path_);
		paras_.swap(rhs.paras_);
		headers_.swap(rhs.headers_);
	}

private:
	Method method_;
	Version version_;
	string path_;
	std::unordered_map<std::string, std::string> paras_;	//²ÎÊý
	std::unordered_map<string, string> headers_;
};
