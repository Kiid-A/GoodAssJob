#include "HttpContext.h"
#include "../include/Buffer.h"

// request line -> header -> context body /cycling
bool HttpContext::parseRequest(Buffer* buf)
{
	bool ok = true;
	bool hasMore = true;
	printf("HttpContext::parseReques:buf:\n%s\n",buf->peek());
	while (hasMore) {
        // RequestLine: POST "some URL string" HTTP/1.1 200 OK
		if (state_ == HttpRequestParseState::ExpectRequestLine) {
            // locate first \r\n
			const char* crlf = buf->findCRLF();
			if (crlf) {
                // parse successfully, then parse URL
				ok = processRequestLine(buf->peek(), crlf);
				if (ok) {
                    // overlook \r\n
					buf->retrieveUntil(crlf + 2);
					state_ = HttpRequestParseState::ExpectHeaders;
				} else {
					hasMore = false;
				}
			} else {
				hasMore = false;
			}
        // Header: XXXX:YYYY
		} else if (state_ == HttpRequestParseState::ExpectHeaders) {
			const char* crlf = buf->findCRLF();
			if (crlf) {
                // find new line --> get header & value then create pair
				const char* colon = std::find(buf->peek(), crlf, ':');
				if (colon != crlf) {
					request_.addHeader(buf->peek(), colon, crlf);
                // colon == crlf means function find return the last value, colon not found
				} else {
					state_ = HttpRequestParseState::ExpectBody;
				}
				buf->retrieveUntil(crlf + 2);
            // crlf not found, no more to read
			} else {
				hasMore = false;
			}
        // Body: a chunk of data
		} else if (state_ == HttpRequestParseState::ExpectBody) {
            // buffer still has data --> that is body/query
			if (buf->readableByte()) {
				request_.setQuery(buf->peek(), buf->writerBegin());
			}
            // has come to an end
			state_ = HttpRequestParseState::GotAll;
			hasMore = false;
		}
	}
	return ok;
}

// RequestLine: Method + URL + Version + CRLF
// URL may has '?' character
bool HttpContext::processRequestLine(const char* begin, const char* end)
{
	bool succeed = true;

	const char* start = begin;
    // seperate string by space
    // start     space     space+1
    // XXXXX                YYYYY/end
	const char* space = std::find(start, end, ' ');
	// deal with Method
	if (space != end && request_.setMethod(start, space)) {
		start = space + 1;
        // find second space, which is URL: path ? requestParameter
		space = std::find(start, end, ' ');
		if (space != end) {
			const char* question = std::find(start, space, '?');
			if (question != space) {
				request_.setPath(start, question);
				request_.setQuery(question, space);
			} else {
                // only have path
				request_.setPath(start, space);
			}
            // overlook space
			start = space + 1;
            // parse Version
			string version(start, end);
			if (version == "HTTP/1.0") {
				request_.setVersion(HttpRequest::Version::Http10);
            } else if (version == "HTTP/1.1") {
				request_.setVersion(HttpRequest::Version::Http11);
            } else {
				succeed = false;
            }
		}
	}
	return succeed;
}
