//
//  InitConnectionHandler.cpp
//  Mp3Player
//
//  Created by henry_xiao on 2023/1/25.
//

#include "InitConnectionHandler.hpp"
#include "Utils/Utils.h"


void writeAllPlayerStates(RapidjsonWriter &writer);
void writeAllMediaLibrary(RapidjsonWriter &writer);

namespace WebSocketHandlers {

InitConnectionHandler::InitConnectionHandler() {
    m_type = TYPE_INIT_CONNECTION;
}

bool keyStringToUint8Array(std::string &input, std::string &output) {
    VecStrings vStrs;

    strSplit(input.c_str(), ',', vStrs);
    assert(vStrs.size() == 32);
    if (vStrs.size() != 32) {
        return false;
    }

    for (auto &s : vStrs) {
        int n = atoi(s.c_str());
        assert(n >= 0 && n < 256);
        output.append(1, char(n));
    }

    return true;
}

/**
 * 弹出窗口询问是否接受远程连接的控制.
 */
void InitConnectionHandler::onMessage(Server *server, const websocketpp::connection_hdl &connection,
                                      uint32_t clientId, const rapidjson::Value &message)
{
    ClientInfo client;
    client.id = clientId;

    // key 是使用',' 分隔的32个数字 1,3,77,64,...
    auto key = getMemberString(message, "key");
    if (!keyStringToUint8Array(key, client.aesKey)) {
        string err = stringPrintf("Invalid client key format: %s", key.c_str());
        DBG_LOG0(err.c_str());
        server->sendErrorMessage(connection, clientId, RC_INVALID_PARAMS, err.c_str());
        return;
    }
    client.latestClientName = getMemberString(message, "clientName");

    mbedtls_aes_setkey_enc(&client.encCtx, (uint8_t *)client.aesKey.c_str(), 256);
    mbedtls_aes_setkey_dec(&client.decCtx, (uint8_t *)client.aesKey.c_str(), 256);

    if (!server->addClient(client)) {
        string err = "Client existed already.";
        DBG_LOG0(err.c_str());
        server->sendErrorMessage(connection, clientId, RC_CLIENT_EXISTED, err.c_str());
        return;
    }

    server->addAuthorizedConnections(connection, clientId);

    {
        rapidjson::StringBuffer buf;
        RapidjsonWriter writer(buf);

        writer.StartObject();
        writer.Key("type");
        writer.String(m_type.c_str());

        writer.Key("result");
        writer.String("OK");
        writer.EndObject();

        server->send(connection, 1, MT_CLIENT_KEY_ENC, buf.GetString(), buf.GetSize());
    }

    {
        rapidjson::StringBuffer buf;
        RapidjsonWriter writer(buf);

        writeAllPlayerStates(writer);
        server->send(connection, 1, MT_CLIENT_KEY_ENC, buf.GetString(), buf.GetSize());
    }

    {
        rapidjson::StringBuffer buf;
        RapidjsonWriter writer(buf);

        writeAllMediaLibrary(writer);
        server->send(connection, 1, MT_CLIENT_KEY_ENC, buf.GetString(), buf.GetSize());
    }
}

} // namespace WebSocketHandlers
