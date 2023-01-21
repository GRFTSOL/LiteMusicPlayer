#ifndef HTTP_REQUEST_HPP
#define HTTP_REQUEST_HPP

#include <string>
#include <vector>


namespace HttpServer {

struct Header {
    std::string                 name;
    std::string                 value;
};

using VecHttpHeaders = std::vector<Header>;

struct Request {
    std::string                 method;
    std::string                 uri;
    int                         versionMajor;
    int                         versionMinor;
    VecHttpHeaders              headers;
    std::string                 body;
};

std::string *getHeaderByName(VecHttpHeaders &headers, const std::string &name);

} // namespace HttpServer

#endif // HTTP_REQUEST_HPP
