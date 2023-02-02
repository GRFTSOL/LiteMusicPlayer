//
//  LocalServer.cpp
//  Mp3Player
//
//  Created by henry_xiao on 2023/1/20.
//

#include "LocalServer.hpp"
#include "Http/StaticFilesHandler.hpp"
#include "WebSocketHandlers/InitConnectionHandler.hpp"
#include "WebSocketHandlers/PlayerRemoteCtrlHandler.hpp"
#include "GenRsaKey.hpp"
#include "../Skin/SkinTypes.h"
#include "../MPlayerUI/Player.h"


using namespace WebSocketHandlers;

LocalServer *LocalServer::_instance = nullptr;

bool loadRSAKeys(cstr_t fnPrivateKey, cstr_t fnPublicKey, mbedtls_pk_context &rsaKey, string &publicKey) {
    string privateKey;
    if (!readFile(fnPrivateKey, privateKey) ||
            !readFile(fnPublicKey, publicKey)) {
        return false;
    }

    mbedtls_ctr_drbg_context ctr_drbg;
    const char *pers = "gen_key";

    mbedtls_pk_init(&rsaKey);
    mbedtls_ctr_drbg_init(&ctr_drbg);

    mbedtls_entropy_context entropy;
    mbedtls_entropy_init(&entropy);

    int ret = mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy,
        (const unsigned char *)pers, strlen(pers));
    if (ret == 0) {
        ret = mbedtls_pk_parse_key(&rsaKey, (uint8_t *)privateKey.c_str(), privateKey.size() + 1,
                                   nullptr, 0, mbedtls_ctr_drbg_random, &ctr_drbg);
        if (ret == 0) {
            mbedtls_pk_context tmp;
            mbedtls_pk_init(&tmp);
            ret = mbedtls_pk_parse_public_key(&tmp, (uint8_t *)publicKey.c_str(), publicKey.size() + 1);
            mbedtls_pk_free(&tmp);
        }
    }

    if (ret != 0) {
        ERR_LOG1("%s\n", getMbedtlsError(ret).c_str());
    }

    mbedtls_ctr_drbg_free(&ctr_drbg);
    mbedtls_entropy_free(&entropy);

    return ret == 0;
}

LocalServer *LocalServer::getInstance() {
    if (!_instance) {
        mbedtls_pk_context rsaKey;

        mbedtls_pk_init(&rsaKey);

        string publicKey;
        string fnPrivateKey = getAppDataDir() + "LocalServerPrivate.key";
        string fnPublicKey = getAppDataDir() + "LocalServerPublic.key";

        if (!loadRSAKeys(fnPrivateKey.c_str(), fnPublicKey.c_str(), rsaKey, publicKey)) {
            string privateKey;
            int err = generateRSAKey(rsaKey, privateKey, publicKey, 2048);
            if (err == 0) {
                printf("%s\n %s\n", privateKey.c_str(), publicKey.c_str());
            }

            writeFile(fnPrivateKey.c_str(), privateKey.c_str());
            writeFile(fnPublicKey.c_str(), publicKey.c_str());
        }

        _instance = new LocalServer(g_profile.getString("LocalServer", "address", "127.0.0.1"),
                                    g_profile.getString("LocalServer", "http_port", "1212"),
                                    g_profile.getString("LocalServer", "web_socket_port", "1213"),
                                    "/Users/henry_xiao/ProjectsPrivate/Mp3Player/LocalServer/www/",
                                    rsaKey, publicKey);

        auto &wsserver = _instance->m_webSocketServer;
        wsserver.registerMessageHandler(make_shared<InitConnectionHandler>());
        wsserver.registerMessageHandler(make_shared<PlayerRemoteCtrlHandler>());
    }

    return _instance;
}

LocalServer::LocalServer(cstr_t address, cstr_t httpPort, cstr_t webSocketPort, cstr_t docRoot, const mbedtls_pk_context &rsaKey, const std::string &publicKey) : m_httpServer(address, httpPort), m_webSocketServer(atoi(webSocketPort), rsaKey, publicKey) {
    auto handler = make_shared<HttpServer::StaticFilesHandler>("/", docRoot);
    m_httpServer.registerRequestHandler(handler);

    m_playerEventSender = make_shared<PlayerEventSender>(&m_webSocketServer);
}

void LocalServer::start() {
    assert(!m_threadHttpServer.isRunning());
    assert(!m_threadWebSocketServer.isRunning());

    m_threadHttpServer.create(httpServerThread, this);

    m_threadWebSocketServer.create(webSocketServerThread, this);
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
