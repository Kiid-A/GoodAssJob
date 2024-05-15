#pragma once
#include "HttpResponse.h"
#include "HttpRequest.h"
#include "HttpServer.h"
#include "string"


namespace HttpMessage
{
    void onRequest(HttpRequest &req, HttpResponse* resp);
} // namespace name


