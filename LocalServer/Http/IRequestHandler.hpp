//
//  IRequestHandler.hpp
//  Mp3Player
//
//  Created by henry_xiao on 2023/1/21.
//

#ifndef IRequestHandler_hpp
#define IRequestHandler_hpp

#include <string>


namespace HttpServer {

class Connection;
using ConnectionPtr = std::shared_ptr<Connection>;

class IRequestHandler {
public:
    virtual ~IRequestHandler() {}

    virtual const std::string &getUriPath() const = 0;
    virtual bool onRequestHeader(const ConnectionPtr &connection) = 0;
    virtual bool onRequestBody(const ConnectionPtr &connection) = 0;

};

using IRequestHandlerPtr = std::shared_ptr<IRequestHandler>;
using VecRequestHandler = std::vector<IRequestHandlerPtr>;

} // namespace HttpServer

#endif /* IRequestHandler_hpp */
