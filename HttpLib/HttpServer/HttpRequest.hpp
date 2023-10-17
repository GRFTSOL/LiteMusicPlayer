//
//  HttpRequest.hpp
//

#pragma once

#ifndef HttpRequest_h
#define HttpRequest_h

#include "../../Utils/UtilsTypes.h"
#include "../../Utils/url.h"
#include "../../Utils/Error.h"
#include "http_parser.h"


class HttpConnection;


enum HttpMethod {
    METHOD_INVALID              = 0,
    METHOD_GET                  = http_method::HTTP_GET,
    METHOD_HEAD                 = http_method::HTTP_HEAD,
    METHOD_POST                 = http_method::HTTP_POST,
    METHOD_PUT                  = http_method::HTTP_PUT,
    METHOD_DELETE               = http_method::HTTP_DELETE,
    METHOD_OPTIONS              = http_method::HTTP_OPTIONS,
};

HttpMethod toHttpMethod(const string &method);

struct HttpRequest {
    HttpMethod                          methodID;
    // string                              method;
    string                              uri;
    int                                 versionMajor;
    int                                 versionMinor;
    ListHttpHeaders                     headers;
    string                              body;
};

string *getHeaderByName(ListHttpHeaders &headers, const string &name);

#endif /* HttpRequest_h */
