#pragma once

#include "HttpRequest.h"

class Buffer;

/*  state machine to parse context/packet according to protocols
*/
class HttpContext
{
public:
    enum class HttpRequestParseState
    {
        ExpectRequestLine,  // default begin state
		ExpectHeaders,
		ExpectBody,
		GotAll,             // end state
    };

private:
    HttpRequestParseState state_;
    HttpRequest request_;

    bool processRequestLine(const char* begin, const char* end);

public:

    HttpContext()
        :state_(HttpRequestParseState::ExpectRequestLine)
    {}

    // parse buffer
    bool parseRequest(Buffer* buf, Timestamp reciveTime);
	bool parseRequest(Buffer* buf);

    bool isAllGot() const { return state_ == HttpRequestParseState::GotAll; }

    // recycle HttpContext
    void reset()
    {
        state_ == HttpRequestParseState::ExpectRequestLine;
        HttpRequest temp;
        request_.swap(temp);
    }

    // when return a reference type, one const and one not is good!
    const HttpRequest& getRequest() const { return request_; }
    HttpRequest& getRequest() { return request_; }
};

