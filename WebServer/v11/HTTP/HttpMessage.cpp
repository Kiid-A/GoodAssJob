#include "HttpMessage.h"


std::string findType(const std::string path)
{
    int index = path.rfind(".");
    return path.substr(index + 1, path.length() - 1);
}

void onRequest(const HttpRequest& req, HttpResponse* resp)
{
    std::string absolutePath = "../data/src" + req.getPath();
    if (req.getPath() == "/") {
        absolutePath += "index.html";
        resp->addHeader("Server", "li");
    }
    printf("absPath: %s\n", absolutePath.c_str());
    std::string type = findType(absolutePath);
    printf("type: %s\n", type.c_str());
    
    if (type == "html") {
        resp->setContentType("text/html");
    } else if (type == "js") {
        resp->setContentType("application/x-javascript");
    } else if (type == "css") {
        resp->setContentType("text/css");
    } else if (type == "jpg") {
        resp->setContentType("image/jpeg");
    } else {
        resp->setStatusCode(HttpResponse::HttpStatusCode::NotFound);
		resp->setStatusMessage("Not Found");
		resp->setCloseConnection(true);
    }
    resp->setBody(absolutePath);
    resp->setStatusCode(HttpResponse::HttpStatusCode::Ok);
    resp->setStatusMessage("OK");
}

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