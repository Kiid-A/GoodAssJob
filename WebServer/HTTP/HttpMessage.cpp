#include "HttpMessage.h"
#include "HttpRequest.h"
#include "../log/Logger.h"

#include <mysql/mysql.h>


using namespace std;




// void onRequest(const HttpRequest& req, HttpResponse* resp)
// {
//     // root dir for test
// 	if (req.getPath() == "/") {
// 		resp->setStatusCode(HttpResponse::HttpStatusCode::Ok);
// 		resp->setStatusMessage("OK");
// 		// resp->setContentType("text/html");
// 		resp->addHeader("Server", "li");
//         resp->setBody("../data/src/index.html");

// 	} else if(req.getPath()=="/favicon.ico") {
// 		resp->setStatusCode(HttpResponse::HttpStatusCode::Ok);
// 		resp->setStatusMessage("OK");
// 		resp->setContentType("image/png");

// 	} else if (req.getPath() == "/hello") {
// 		resp->setStatusCode(HttpResponse::HttpStatusCode::Ok);
// 		resp->setStatusMessage("OK");
// 		resp->setContentType("text/plain");

// 	} else {
// 		resp->setStatusCode(HttpResponse::HttpStatusCode::NotFound);
// 		resp->setStatusMessage("Not Found");
// 		resp->setCloseConnection(true);
// 	}
// }