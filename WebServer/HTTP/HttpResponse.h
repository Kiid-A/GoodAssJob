#pragma once

#include <unordered_map>
#include <string>
#include <fstream>
#include <iostream>
#include <filesystem>

using std::string;
class Buffer;


/*  HttpResponse
    Add INFO in response: request line + Status
*/
class HttpResponse
{
public:
    enum HttpStatusCode
    {
        Unknown,
        Ok = 200,
        Accepted = 201,
        MovedPermanently = 301,
        BadRequest = 400,
        NotFound = 404,
    };

private:
    std::unordered_map<string, string> headers_;
    HttpStatusCode  statusCode_;
    string statusMessage_;
    bool isConnectionClosed_;
    string body_;

public:
    explicit HttpResponse(bool close)
        :statusCode_(HttpStatusCode::Unknown),
        isConnectionClosed_(close)
    {}

    // identify the state of dir
    void setStatusCode(HttpStatusCode code) 
    { statusCode_ = code; }
    
    // set message, which will be parsed by ..
    void setStatusMessage(const string& message) 
    { statusMessage_ = message; }
    
    // close it
    void setCloseConnection(bool on) 
    { isConnectionClosed_ = on; }
    
    /* @return whether connection is closed */
    bool isConnectionClosed() const 
    { return isConnectionClosed_; }
    
    /* set k-v pair */
    void setContentType(const string& contentType) 
    { addHeader("Content-Type", contentType); }

    void addHeader(const string& key, const string& value) 
    { headers_[key] = value; }
    
    /* set HTML body */
    void setBody(string filePath)
    {   
        string buf;
        std::ifstream infile;
        infile.open(filePath, std::ios::in);
        while (getline(infile, buf)) {
            body_ += buf + "\r\n";
        }
    }
    // append response to buffer
    void appendToBuffer(Buffer* output) const;
};