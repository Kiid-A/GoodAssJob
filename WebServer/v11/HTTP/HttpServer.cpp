#include "HttpServer.h"
#include "HttpContext.h"
#include "HttpRequest.h"
#include "HttpResponse.h"

void defaultHttpCallBack(const HttpRequest& req,HttpResponse* resp)
{
    resp->setStatusCode(HttpResponse::HttpStatusCode::NotFound);
	resp->setStatusMessage("Not Found");
	resp->setCloseConnection(true);
}

HttpServer::HttpServer(EventLoop* loop, const InetAddr& listenAddr)
	:server_(listenAddr, loop),
	httpCallBack_([](const HttpRequest& req, HttpResponse* resp) { defaultHttpCallBack(req, resp); })
{
	server_.setConnectionCallBack([this](const ConnectionPtr& conn) { onConnection(conn); });
	server_.setMessageCallBack([this](const ConnectionPtr& conn, Buffer* buf) { onMessage(conn, buf); });
}

void HttpServer::start(int numOfThreads)
{
	server_.start(numOfThreads);
}

// construct a context when connection comes
void HttpServer::onConnection(const ConnectionPtr& conn)
{
    if(conn->isConnected()) {
        conn->setContext(HttpContext());
    }
}

// call conn->context_ to parse packet
void HttpServer::onMessage(const ConnectionPtr& conn, Buffer* buf)
{
    auto context = std::any_cast<HttpContext>(conn->getContext());
    // receive nothing
	if (!context) {
		printf("context kong...\n");
		LOG_ERROR<<"context is bad\n";
		return;
	}
    // receive bad context
	if (!context->parseRequest(buf)) {
		conn->send("HTTP/1.1 400 Bad Request\r\n\r\n");
		conn->shutdown();
		printf("http 400\n");
	}
    // all got
	if (context->isAllGot()) {
		onRequest(conn, context->getRequest());
        // once request done, we unbind connection and HttpContext for duplicate usage
		context->reset();
	}
}

void HttpServer::onRequest(const ConnectionPtr& conn, const HttpRequest& req)
{
	const std::string& connetion = req.getHeader("Connection");
    // Http1.0 short connection/Http1.1 long connection
	bool close = connetion == "close" || (req.getVersion() == HttpRequest::Version::Http10 && connetion != "Keep-Alive");

	HttpResponse response(close);
	httpCallBack_(req, &response);

	Buffer buf;
	response.appendToBuffer(&buf);
	conn->send(&buf);
	if (response.isConnectionClosed()) {
		conn->shutdown();
	}
}

