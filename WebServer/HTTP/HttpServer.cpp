#include "HttpServer.h"

void defaultHttpCallBack(const HttpRequest& req,HttpResponse* resp)
{
    resp->setStatusCode(HttpResponse::HttpStatusCode::NotFound);
	resp->setStatusMessage("Not Found");
	resp->setCloseConnection(true);
}

HttpServer::HttpServer(EventLoop* loop, const InetAddr& listenAddr, int threadNum, 
					   string user, string passwd, string dbName, int sqlNum)
	:server_(listenAddr, loop),
	httpCallBack_([](const HttpRequest& req, HttpResponse* resp) { defaultHttpCallBack(req, resp); }),
	threadNum_(threadNum),
	user_(user),
	passwd_(passwd),
	dbName_(dbName),
	sqlNum_(sqlNum)
{
	server_.setConnectionCallBack([this](const ConnectionPtr& conn) { onConnection(conn); });
	server_.setMessageCallBack([this](const ConnectionPtr& conn, Buffer* buf) { onMessage(conn, buf); });
}

void HttpServer::sqlPool()
{
	connPool_ = ConnectionPool::getInstance();
	connPool_->init("localhost", user_, passwd_, dbName_, 3306, sqlNum_);

	/* future upd */
	// void initMysqlResult();
	initMysqlResult();
}

void HttpServer::initMysqlResult()
{
	MYSQL *mysql = nullptr;
	ConnectionRAII mysqlConn(&mysql, connPool_);

	if (mysql_query(mysql, "SELECT username,passwd FROM user")) {
        LOG_ERROR << "SELECT error:%s\n" << mysql_error(mysql);
    }

    MYSQL_RES *result = mysql_store_result(mysql);

    int numFields = mysql_num_fields(result);

    MYSQL_FIELD *fields = mysql_fetch_fields(result);

    while (MYSQL_ROW row = mysql_fetch_row(result)) {
        string temp1(row[0]);
        string temp2(row[1]);
        users[temp1] = temp2;
		LOG_INFO << "user: " << temp1 << " passwd: " << temp2;
    }
}

void HttpServer::eventLoop()
{
	server_.loop();
}

void HttpServer::threadPool()
{
	server_.start(threadNum_);
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
		LOG_ERROR << "context is bad\n";
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

string findType(const string path)
{
    int index = path.rfind(".");
    return path.substr(index + 1, path.length() - 1);
}

string getAction(const string path) 
{
    int index = path.rfind(".");
    return path.substr(1, index - 1);
}

void locate(HttpRequest &req, HttpResponse *resp)
{
    string absolutePath = "./data/src" + req.getPath();
	if (req.getPath() == "/") {
		absolutePath += "judge.html";
		resp->addHeader("Server", "li");
	}

	printf("absPath: %s\n", absolutePath.c_str());
	string type = findType(absolutePath);
	printf("type: %s\n", type.c_str());

	if (type == "html") {
		resp->setContentType("text/html");
	}
	else if (type == "js") {
		resp->setContentType("application/x-javascript");
	}
	else if (type == "css") {
		resp->setContentType("text/css");
	}
	else if (type == "jpg") {
		resp->setContentType("image/jpeg");
	} else if (type == "mp4") {
		resp->setContentType("video/mpeg4");
	}
	else {
		resp->setStatusCode(HttpResponse::HttpStatusCode::NotFound);
		resp->setStatusMessage("Not Found");
		resp->setCloseConnection(true);
	}
	resp->setBody(absolutePath);
	resp->setStatusCode(HttpResponse::HttpStatusCode::Accepted);
	resp->setStatusMessage("OK");
}


pair<string, string> parseUser(const string query)
{
	int userIdx = query.find_first_of('=');
    int sep = query.find_first_of('&');
    int passwdIdx = query.find_last_of('=');
    int len = sep - userIdx;

    string name = query.substr(userIdx + 1, len - 1);
    string passwd = query.substr(passwdIdx + 1, query.length() - 1);

    LOG_INFO << "user: " << name << " passwd: " << passwd;

	return pair<string, string> {name, passwd};
}

/* format: user=****&passwd=**** */
void HttpServer::registerHandler(HttpRequest& req, const string query)
{
    pair<string, string> user = parseUser(query);
	string name = user.first;
	string passwd = user.second;

    /* future update: add sql */
    char *sql_insert = (char *)malloc(sizeof(char) * 200);
            strcpy(sql_insert, "INSERT INTO user(username, passwd) VALUES(");
            strcat(sql_insert, "'");
            strcat(sql_insert, name.c_str());
            strcat(sql_insert, "', '");
            strcat(sql_insert, passwd.c_str());
            strcat(sql_insert, "')");

            if (users.find(name) == users.end())
            {
                mutex_.lock();
                int res = mysql_query(mysql_, sql_insert);
                users.insert(pair<string, string>(name, passwd));
                mutex_.unlock();

                if (!res) {
                    req.setPath("/log.html");
                } else {
                    req.setPath("/registerError.html");
				}
            } else {
                req.setPath("/registerError.html");
			}
}

void HttpServer::logHandler(HttpRequest& req, const string query)
{
	auto user = parseUser(query);	
	string name = user.first;
	string passwd = user.second;

	if (users.find(name) != users.end() && users[name] == passwd) {
        req.setPath("/welcome.html");
	} else {
        req.setPath("/logError.html");
	}
}

void HttpServer::onRequest(const ConnectionPtr& conn, HttpRequest& req)
{
	const std::string& connetion = req.getHeader("Connection");
    // Http1.0 short connection/Http1.1 long connection
	bool close = connetion == "close" || (req.getVersion() == HttpRequest::Version::Http10 && connetion != "Keep-Alive");

	HttpResponse response(close);
	// printf("method: %s\n", req.methodString());
    LOG_INFO << req.methodString();
    LOG_INFO << req.getQuery();

    switch (req.getMethod())
    {
    case HttpRequest::Method::Get:
    {
        locate(req, &response);
        break;
    }

    case HttpRequest::Method::Post:
    {
        string type = findType(req.getPath());
        string query = req.getQuery();
        if (type == "action") {
            string action = getAction(req.getPath());
			LOG_INFO << "present action: " << action;
            if (action == "register") {
                registerHandler(req, query);
                // req.setPath("/welcome.html");
            } else if (action == "log") {
				logHandler(req, query);
			}
        } else {}
        locate(req, &response);
        break;
    }
    default:
        break;
    }

	Buffer buf;
	response.appendToBuffer(&buf);
	conn->send(&buf);

	if (!isInit_) {
		isInit_ = true;
		init(conn, req);
	}

	if (response.isConnectionClosed()) {
		conn->shutdown();
	}
}

vector<string> files = 
{
	"favicon.ico",
	"login.gif",
	"loginnew.gif",
	"picture.gif",
	"register.gif",
	"test1.jpg",
	"video.gif",
	"xxx.jpg",
	"xxx.mp4"
};

void HttpServer::init(const ConnectionPtr& conn, HttpRequest& req)
{
	for (auto file : files) {
		HttpResponse resp(false);
		req.setPath(file);
		locate(req, &resp);
		
		Buffer buf;

		resp.appendToBuffer(&buf);
		conn->send(&buf);
	}
}