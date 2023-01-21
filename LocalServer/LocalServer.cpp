//
//  LocalServer.cpp
//  Mp3Player
//
//  Created by henry_xiao on 2023/1/20.
//

#include <assert.h>
#include "LocalServer.hpp"
#include "Http/StaticFilesHandler.hpp"
#include "../Skin/SkinTypes.h"


LocalServer *LocalServer::_instance = nullptr;

LocalServer *LocalServer::getInstance() {
    if (!_instance) {
        _instance = new LocalServer(g_profile.getString("LocalServer", "address", "127.0.0.1"),
                                    g_profile.getString("LocalServer", "http_port", "1212"),
                                    g_profile.getString("LocalServer", "web_socket_port", "1213"),
                                    "/Users/henry_xiao/ProjectsPrivate/Mp3Player/LocalServer/www/");
    }

    return _instance;
}

LocalServer::LocalServer(cstr_t address, cstr_t httpPort, cstr_t webSocketPort, cstr_t docRoot) : m_httpServer(address, httpPort), m_webSocketServer(atoi(webSocketPort)) {
    auto handler = make_shared<HttpServer::StaticFilesHandler>("/", docRoot);
    m_httpServer.registerRequestHandler(handler);
}

void LocalServer::start() {
    assert(!m_threadHttpServer.isRunning());
    assert(!m_threadWebSocketServer.isRunning());

    m_threadHttpServer.create(httpServerThread, this);
}

void LocalServer::httpServerThread(void *param) {
    LocalServer *server = (LocalServer *)param;

    try {
        server->m_httpServer.run();
    } catch (std::exception& e) {
        ERR_LOG1("Exception in httpServerThread: %s", e.what());
    }
}

void LocalServer::stop() {
    
}

void LocalServer::webSocketServerThread(void *param) {
    LocalServer *server = (LocalServer *)param;

    try {
        server->m_webSocketServer.run();
    } catch (websocketpp::exception const &e) {
        ERR_LOG1("WebSocket Exception(1): %s\n", e.what());
    } catch (std::exception &e) {
        ERR_LOG1("WebSocket Exception(2): %s\n", e.what());
    }
}
