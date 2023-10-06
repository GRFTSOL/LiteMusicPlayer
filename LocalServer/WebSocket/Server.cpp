//
//  Server.cpp
//  Mp3Player
//
//  Created by henry_xiao on 2023/1/21.
//

#include "Server.hpp"
#include "Utils/Utils.h"
#include "rapidjson/reader.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/document.h"
#include "rapidjson/error/error.h"
#include "rapidjson/error/en.h"


using namespace rapidjson;

std::string getMbedtlsError(int err);

namespace WebSocket {

struct DataPacket {
    MessageType                 type;
    uint32_t                    clientId;
    string                      data;

    bool parse(const uint8_t *data, size_t len);

};

const int CUR_VERSION = 1;
const int BLOCK_SIZE = 16;

void fillRandom(uint8_t *arr, size_t size) {
    for (int i = 0; i < (int)size; i++) {
        arr[i] = rand() % 256;
    }
}

bool DataPacket::parse(const uint8_t *data, size_t len) {
    BinaryInputStream stream(data, len);
    try {
        uint8_t version = stream.readUInt8();
        if (version != CUR_VERSION) {
            DBG_LOG1("Failed to parse DataPacket, incorrect Version: %d", version);
            return false;
        }

        type = (MessageType)stream.readUInt8();
        clientId = stream.readUInt32BE();
        this->data.assign((cstr_t)stream.currentPtr(), stream.remainingSize());
    } catch (BinaryStreamOutOfRange e) {
        DBG_LOG1("Failed to parse DataPacket: %s", e.what());
        return false;
    }

    return true;
}

string packDataPacket(MessageType type, uint32_t clientId, const string &data) {
    BinaryOutputStream stream;

    stream.writeUInt8(CUR_VERSION);
    stream.writeUInt8(type);
    stream.writeUInt32BE(clientId);
    auto s = stream.toStringView();

    string packet((cstr_t)s.data, s.len);
    packet.append(data);
    return packet;
}

bool parsePacketJsonData(const string &data, rapidjson::Document &doc, string &type) {
    ParseResult ok = doc.Parse(data.c_str(), data.size());
    if (!ok) {
        DBG_LOG3("JSON parse error: %s (offset: %l), %s\n",
            rapidjson::GetParseError_En(ok.Code()),
            (unsigned int)ok.Offset(), data.c_str() + ok.Offset());
        return false;
    }

    assert(doc.IsObject());
    if (!doc.IsObject()) {
        return false;
    }

    auto itType = doc.FindMember("type");
    if (itType == doc.MemberEnd()) {
        DBG_LOG1("No 'type' for message: %s", data.c_str());
        return false;
    }

    auto &typeVal = (*itType).value;
    if (!typeVal.IsString()) {
        DBG_LOG1("'type' is not String, for message: %s", data.c_str());
        return false;
    }

    DBG_LOG1("Handle Websocket message: %s", typeVal.GetString());
    type.assign(typeVal.GetString(), typeVal.GetStringLength());

    return true;
}

static void randomPadding(string &out) {
    size_t length = out.size() % BLOCK_SIZE;
    if (length != 0) {
        length = BLOCK_SIZE - length;

        uint8_t iv[BLOCK_SIZE];
        fillRandom(iv, length);
        out.append((char *)iv, length);
    }
}

bool ClientInfo::encrypt(const char *input, size_t len, std::string &output) {
    assert(aesKey.size() == 32);

    uint8_t iv[BLOCK_SIZE];

    string buf;

    // crc
    uint32ToBE((uint32_t)crc32(0, (uint8_t *)input, (uint32_t)len), iv); // 使用 iv 来临时存储
    buf.append((char *)iv, 4);

    // len
    uint32ToBE((uint32_t)len, iv); // 使用 iv 来临时存储
    buf.append((char *)iv, 4);

    // data
    buf.append(input, len);
    randomPadding(buf);

    fillRandom(iv, sizeof(iv));

    output.append((char *)iv, BLOCK_SIZE);
    output.resize(buf.size() + BLOCK_SIZE);
    int ret = mbedtls_aes_crypt_cbc(&encCtx, MBEDTLS_AES_ENCRYPT, buf.size(), iv,
        (uint8_t *)buf.data(), (uint8_t *)output.data() + BLOCK_SIZE);

    return ret == 0;
}

bool ClientInfo::decrypt(const char *input, size_t len, std::string &output) {
    assert(aesKey.size() == 32);
    assert(len >= 32 && len % BLOCK_SIZE == 0);
    if (len < 32 || len % BLOCK_SIZE != 0) {
        return false;
    }

    uint8_t *iv = (uint8_t *)input;
    input += BLOCK_SIZE;
    len -= BLOCK_SIZE;

    output.resize(len);
    int ret = mbedtls_aes_crypt_cbc(&decCtx, MBEDTLS_AES_DECRYPT, len, iv,
        (uint8_t *)input, (uint8_t *)output.data());
    if (ret != 0) {
        return false;
    }

    const uint8_t *p = (uint8_t *)output.c_str();
    uint32_t crc = uint32FromBE(p); p += 4;
    uint32_t lenActual = uint32FromBE(p); p += 4;
    output.erase(output.begin(), output.begin() + ((cstr_t)p - output.c_str()));
    if (lenActual <= output.size()) {
        output.resize(lenActual);
        uint32_t crcActual = (uint32_t)crc32(0, (uint8_t *)output.c_str(), lenActual);
        if (crc != crcActual) {
            DBG_LOG2("CRC32 is not correct: %s vs %d", crc, crcActual);
            return false;
        }
    } else {
        DBG_LOG2("Invalid length: %d vs %d", lenActual, (uint32_t)output.size());
        return false;
    }

    return true;
}

void IMessageHandler::sendResult(Server *server, const websocketpp::connection_hdl &connection,
    uint32_t clientId, const char *code, const char *message)
{
    rapidjson::StringBuffer buf;
    RapidjsonWriter writer(buf);

    writer.StartObject();
    writer.Key("type");
    writer.String(m_type.c_str());

    writer.Key("result");
    writer.String(code);

    if (message) {
        writer.Key("message");
        writer.String(message);
    }

    writer.EndObject();

    server->send(connection, 1, MT_CLIENT_KEY_ENC, buf.GetString(), buf.GetSize());
}

Server::Server(int port, const mbedtls_pk_context &rsaKey, const std::string &publicKey) : m_port(port), m_rsaKey(rsaKey), m_publicKey(publicKey)
{
    mbedtls_ctr_drbg_init(&m_ctr_drbg);

    mbedtls_entropy_init(&m_entropy);

    const char *pers = "ws_server";

    int ret = mbedtls_ctr_drbg_seed(&m_ctr_drbg, mbedtls_entropy_func, &m_entropy,
        (const unsigned char *)pers, strlen(pers));
    assert(ret == 0);
}

Server::~Server() {
    mbedtls_pk_free(&m_rsaKey);
    mbedtls_ctr_drbg_free(&m_ctr_drbg);
    mbedtls_entropy_free(&m_entropy);
}

void Server::processMessage(const connection_hdl &connection, uint32_t clientId,
                            const std::string &type, rapidjson::Document &json) {
    auto it = m_mapHandlers.find(type.c_str());
    if (it == m_mapHandlers.end()) {
        DBG_LOG1("NO handler, for message: %s", type.c_str());
        return;
    }

    DBG_LOG1("Handle Websocket message: %s", type.c_str());

    (*it).second->onMessage(this, connection, clientId, json);
}

void Server::onMessage(connection_hdl hdl, message_ptr msg) {
    auto &payload = msg->get_payload();

    DataPacket packet;
    if (!packet.parse((uint8_t *)payload.c_str(), payload.size())) {
        return;
    }

    if (packet.type == MT_GET_PUBLIC_KEY) {
        // Client 请求 public key
        rapidjson::StringBuffer buf;
        RapidjsonWriter writer(buf);

        writer.StartObject();
        writer.Key("type");
        writer.String(TYPE_PUBLIC_KEY);

        writer.Key("pub_key");
        writer.String(getPublicKey().c_str());
        writer.EndObject();

        send(hdl, 1, MT_PUBLIC_KEY, buf.GetString(), buf.GetSize());
    } else if (packet.type == MT_PUB_KEY_ENC) {
        // Client 使用 public key 加密的数据.

        string result;
        result.resize(packet.data.size() + 16);
        size_t len = 0;

        int ret = mbedtls_pk_decrypt(&m_rsaKey, (uint8_t *)packet.data.c_str(),
            packet.data.size(), (uint8_t *)result.data(), &len, result.capacity(),
            mbedtls_ctr_drbg_random, &m_ctr_drbg);
        if (ret != 0) {
            DBG_LOG2("mbedtls_pk_decrypt returned -0x%04x, %s\n", -ret, getMbedtlsError(ret).c_str());
            return;
        }
        result.resize(len);

        rapidjson::Document doc;
        string type;

        if (!parsePacketJsonData(result, doc, type)) {
            return;
        }

        if (type != TYPE_INIT_CONNECTION) {
            // Public key 只允许初始化链接的命令.
            DBG_LOG1("Invalid message type: %s", type.c_str());
            return;
        }

        // 新的连接
        processMessage(hdl, packet.clientId, type, doc);
    } else if (packet.type == MT_CLIENT_KEY_ENC) {
        // Client 使用 client key 加密数据.
        ClientInfoPtr client = getClient(packet.clientId);
        if (!client) {
            sendErrorMessage(hdl, packet.clientId, RC_NOT_INITED, "Client is NOT authorized by user.");
            return;
        }
        std::string data;
        if (!client->decrypt(packet.data.c_str(), packet.data.size(), data)) {
            sendErrorMessage(hdl, packet.clientId, RC_FAILED_DECRYPT, "Failed to decrypt with client Key.");
            return;
        }

        rapidjson::Document doc;
        string type;

        if (!parsePacketJsonData(data, doc, type)) {
            return;
        }

        // 新的连接
        processMessage(hdl, packet.clientId, type, doc);
    }
}

void Server::onCloseHandler(connection_hdl hdl) {
    RMutexAutolock autolock(m_mutex);

    auto it = m_authorizedConnections.find(hdl);
    if (it == m_authorizedConnections.end()) {
        return;
    }

    m_authorizedConnections.erase(it);
}

bool Server::addClient(const ClientInfo &info) {
    RMutexAutolock autolock(m_mutex);
    m_mapClients[info.id] = make_shared<ClientInfo>(info);
    return true;
}

ClientInfoPtr Server::getClient(uint32_t clientId) {
    RMutexAutolock autolock(m_mutex);
    auto it = m_mapClients.find(clientId);
    if (it == m_mapClients.end()) {
        return nullptr;
    }

    return (*it).second;
}

void Server::addAuthorizedConnections(const connection_hdl &connection, uint32_t clientId) {
    RMutexAutolock autolock(m_mutex);
    m_authorizedConnections[connection] = clientId;
}

void Server::sendToAllAhtorizedClients(const char *payload, size_t lenPayload, uint32_t permissions) {
    m_mutex.lock();
    auto connections = m_authorizedConnections;
    auto clients = m_mapClients;
    m_mutex.unlock();

    for (auto &item : connections) {
        auto it = clients.find(item.second);
        if (it != clients.end()) {
            auto &client = (*it).second;
            if ((client->permissions & permissions) == permissions) {
                send(item.first, client->id, MT_CLIENT_KEY_ENC, payload, lenPayload);
            }
        }
    }
}

void Server::send(const connection_hdl &connection, uint32_t clientId,
    MessageType type, const char *payload, size_t lenPayload)
{
    string packet;
    if (type == MT_PUBLIC_KEY || type == MT_ERROR) {
        packet = packDataPacket(type, clientId, payload);
    } else {
        assert(type == MT_CLIENT_KEY_ENC);
        auto client = getClient(clientId);
        if (!client) {
            sendErrorMessage(connection, clientId, RC_NOT_INITED, "Client is NOT authorized by user.");
            return;
        }

        string encrypted;
        if (!client->encrypt(payload, lenPayload, encrypted)) {
            sendErrorMessage(connection, clientId, RC_FAILED_ENCRYPT, "Failed to call encryption API.");
            return;
        }

        packet = packDataPacket(MT_CLIENT_KEY_ENC, clientId, encrypted);
    }

    try {
        m_server.send(connection, packet, websocketpp::frame::opcode::binary);
    } catch (websocketpp::exception const & e) {
        DBG_LOG1("Failed to send: ", e.what());
    }
}

void Server::sendErrorMessage(const connection_hdl &connection,
    uint32_t clientId, const char *errCode, const char *desc)
{
    rapidjson::StringBuffer buf;
    RapidjsonWriter writer(buf);

    writer.StartObject();
    writer.Key("code");
    writer.String(errCode);

    writer.Key("description");
    writer.String(desc);
    writer.EndObject();

    send(connection, clientId, MT_ERROR, buf.GetString(), buf.GetSize());
}

void Server::run() {
    // Set logging settings
    m_server.clear_access_channels(websocketpp::log::alevel::all);
    m_server.set_access_channels(websocketpp::log::alevel::fail);
    // m_server.set_access_channels(websocketpp::log::alevel::all);
    // m_server.clear_access_channels(websocketpp::log::alevel::frame_payload);

    m_server.init_asio();

    m_server.set_message_handler([this](connection_hdl hdl, message_ptr msg) {
        this->onMessage(hdl, msg);
    });
    m_server.set_close_handler([this](connection_hdl hdl) {
        this->onCloseHandler(hdl);
    });

    m_server.listen(m_port);

    m_server.start_accept();

    m_server.run();
}

void Server::registerMessageHandler(const IMessageHandlerPtr &handler) {
    assert(m_mapHandlers.find(handler->getMessageType()) == m_mapHandlers.end());
    m_mapHandlers[handler->getMessageType()] = handler;
}

} // namespace WebSocket
