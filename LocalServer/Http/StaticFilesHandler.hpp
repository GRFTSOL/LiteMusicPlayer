#pragma once

#ifndef Http_StaticFilesHandler_hpp
#define Http_StaticFilesHandler_hpp

#include "IRequestHandler.hpp"


namespace HttpServer {

class StaticFilesHandler : public IRequestHandler {
public:
    StaticFilesHandler(const std::string &uriPath, const std::string &pathOrFile);

    virtual const std::string &getUriPath() const override { return m_uriPath; }
    virtual bool onRequestHeader(const ConnectionPtr &connection) override;
    virtual bool onRequestBody(const ConnectionPtr &connection) override;

protected:
    std::string                     m_uriPath;
    std::string                     m_pathOrFile;

};

} // namespace HttpServer

#endif
