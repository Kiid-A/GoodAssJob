#pragma once
#include "HttpResponse.h"
#include "HttpRequest.h"
#include "string"


void onRequest(const HttpRequest& req, HttpResponse* resp);
