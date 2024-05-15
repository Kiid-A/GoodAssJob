#pragma once

#include "../include/Timestamp.h"
#include <string>
#include <unordered_map>

using std::string;

/* HttpRequest: parse header and CRUD
*/
class HttpRequest
{
public:
    // HTTP Method
    enum class Method
    {
        Invalid,
        Get, 
        Post, 
        Head, 
        Put, 
        Delete,
    };

    enum class Version
    {
        Unknown,
        Http10,
        Http11,
    };

private:
    // Http method
    Method method_;
    // Http version
    Version version_;
    // request path
    string path_;
    // request body
    string query_;

    Timestamp receiveTime_;
    // header types and corresponding values
    std::unordered_map<string, string> headers_;

public:
    HttpRequest()
        :method_(Method::Invalid),
        version_(Version::Unknown)
    {}

    void setVersion(Version v) { version_ = v; }
    Version getVersion() const { return version_; }

    // scan the request and march string
    // @return if method is valid
    bool setMethod(const char* begin, const char* end)
    {
        string m(begin, end);
        if(m == "GET") {
            method_ = Method::Get;
        } else if(m == "POST") {
            method_ = Method::Post;
        } else if(m == "HEAD") {
            method_ = Method::Head;
        } else if(m == "PUT") {
            method_ = Method::Put;
        } else if(m == "DELETE") {
            method_ = Method::Delete;
        } else {
            method_ = Method::Invalid;
        }
        return method_ != Method::Invalid;
    }

    Method getMethod() const { return method_; }

    const char* methodString()const {
		const char* result = "UNKNOWN";
		switch (method_) {
		case Method::Get:
			result = "GET";
			break;
		case Method::Post:
			result = "POST";
			break;
		case Method::Head:
			result = "PUT";
			break;
		case Method::Delete:
			result = "DELETE";
			break;
		default:
			break;
		}
		return result;
	}

    void setPath(string url)
    {
        path_.assign(url.begin(), url.end());
    }

    void setPath(const char* begin, const char* end)
    {
        path_.assign(begin, end);
    }

    const string& getPath() const { return path_; }

    void setQuery(const char* begin, const char* end)
    {
        query_.assign(begin, end);
    }

    const string& getQuery() const { return query_; }

    void setReceiveTime(Timestamp t) { receiveTime_ = t; }

    Timestamp getReceiveTime() const { return receiveTime_; }

    // header format: XXXX:YYYY, thus seperating the 2 sides of colon
    void addHeader(const char* begin, const char* colon, const char* end)
	{
		string field(begin, colon);
		++colon;
        // filter space after colon
		while (colon < end && isspace(*colon))
			++colon;

        // filter space in value
		string value(colon, end);
		auto size = value.size();
        while (!value.empty() && isspace(value[size - 1]))
			value.resize(size - 1);

		headers_[field] = value;
	}
    
    // @return value's couterpart header
    string getHeader(const string& field) const
    {
        string result;
		auto it = headers_.find(field);
        // end() is like '\0' in string
		if (it != headers_.end()) {
			return it->second;
		}
		return result;
    }

    // swap 2 HttpRequest
    void swap(HttpRequest& that)
	{
		std::swap(method_, that.method_);
		std::swap(version_, that.version_);
		path_.swap(that.path_);
		query_.swap(that.query_);
		//receiveTime_.swap(that.receiveTime_);
		headers_.swap(that.headers_);
	}

    const std::unordered_map<string, string>& getHeaders() const { return headers_; }
};


