#include "HttpResponse.h"
#include "../include/Buffer.h"

void HttpResponse::appendToBuffer(Buffer* output) const
{
	// add request line
	string buf = "HTTP/1.1 " + std::to_string(static_cast<int>(statusCode_));
	output->append(buf);
	output->append(statusMessage_);
	output->append("\r\n");

	// add request header
	if (isConnectionClosed_) {
		output->append("Connection: close\r\n");
	} else {
		buf = "Content-Length:" + std::to_string(body_.size()) + "\r\n";
		output->append(buf);
		output->append("Connection: Keep-Alive\r\n");
	}

	// fetch header
	for (const auto& header : headers_) {
		buf = header.first + ": " + header.second + "\r\n";
		output->append(buf);
	}

	// empty line
	output->append("\r\n");	
	// response body
	output->append(body_);	
}
