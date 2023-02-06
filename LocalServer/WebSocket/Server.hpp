//
//  Server.hpp
//  Mp3Player
//
//  Created by henry_xiao on 2023/1/21.
//

#ifndef Server_hpp
#define Server_hpp

#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>
#include <unordered_map>
#include "mbedtls/build_info.h"
#include "mbedtls/platform.h"
#include "mbedtls/error.h"
#include "mbedtls/pk.h"
#include "mbedtls/ecdsa.h"
#include "mbedtls/rsa.h"
#include "mbedtls/error.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "Utils/rapidjson.h"


namespace WebSocket {

using WsServer = websocketpp::server<websocketpp::config::asio>;
using connection_hdl = websocketpp::connection_hdl;
class Server;

class IMessageHandler {
public:
    virtual ~IMessageHandler() {}

    virtual const std::string &getMessageType() { return m_type; }
    virtual void onMessage(Server *server, const websocketpp::connection_hdl &connection, uint32_t clientId, const rapidjson::Value &message) = 0;

    void sendResult(Server *server, const websocketpp::connection_hdl &connection,
                    uint32_t clientId, const char *code, const char *message = nullptr);

    std::string                 m_type;

};

using IMessageHandlerPtr = std::shared_ptr<IMessageHandler>;
using VecIMessageHandlers = std::vector<IMessageHandlerPtr>;
using MapIMessageHandlers = std::unordered_map<std::string, IMessageHandlerPtr>;

enum MessageType {
    MT_ERROR                    = 0,
    MT_GET_PUBLIC_KEY           = 1,
    MT_PUBLIC_KEY               = 2,
    MT_PUB_KEY_ENC              = 3,
    MT_CLIENT_KEY_ENC           = 4,
};

#define TYPE_PUBLIC_KEY             "PublicKey"
#define TYPE_INIT_CONNECTION        "InitConnection"
#define TYPE_PLAYER_REMOTE_CTRL     "PlayerRemoteCtrl"
#define TYPE_PLAYER_NOTIFICATION    "PlayerNotification"
#define TYPE_PLAYER_STATES          "PlayerStates"
#define TYPE_MEDIA_LIB_NOTIFICATION "MediaLibNotification"
#define TYPE_MEDIA_LIB_ALL          "MediaLibAll"

enum Permissions {
    P_PLAYER_CTRL               = 1,
    P_R_MEDIA_LIB               = 1 << 1,
    P_W_MEDIA_LIB               = 1 << 2,
    P_R_LYRICS                  = 1 << 3,
    P_W_LYRICS                  = 1 << 4,
    P_ALL                       = 0xFFFFFFF,
};

// Result Code:
#define RC_OK                   "OK"
#define RC_NOT_INITED           "NOT_INITED"        // 未初始化
#define RC_NOT_AUTHORIZED       "NOT_AUTHORIZED"    // 未授权/无权限
#define RC_FAILED_ENCRYPT       "FAILED_ENCRYPT"    // 加密失败
#define RC_FAILED_DECRYPT       "FAILED_DECRYPT"    // 解密失败
#define RC_INVALID_PARAMS       "INVALID_PARAMS"    // 参数不正确
#define RC_CLIENT_EXISTED       "CLIENT_EXISTED"    // 客户端已经存在

/**
 * 连接的 客户端信息
 */
struct ClientInfo {
    uint32_t                    id;
    uint32_t                    permissions = P_ALL;

    // 在执行关键命令时需要 client 传递的 cmdID 必须大于此值，防止重放攻击.
    uint32_t                    nextCmdId = 0;

    std::string                 latestIp;
    std::string                 latestClientName;
    std::string                 aesKey;
    time_t                      latestConnTime = 0;

    mbedtls_aes_context         encCtx, decCtx;

    bool encrypt(const char *input, size_t len, std::string &output);
    bool decrypt(const char *input, size_t len, std::string &output);

};

using ClientInfoPtr = std::shared_ptr<ClientInfo>;

class Server {
public:
    using message_ptr = WsServer::message_ptr;

    Server(int port, const mbedtls_pk_context &rsaKey, const std::string &publicKey);
    virtual ~Server();

    void run();
    void registerMessageHandler(const IMessageHandlerPtr &handler);

    void send(const connection_hdl &connection, uint32_t clientId,
              MessageType type, const char *payload, size_t lenPayload);
    void sendErrorMessage(const connection_hdl &connection, uint32_t clientId, const char *errCode, const char *desc);
    void processMessage(const connection_hdl &connection, uint32_t clientId, const std::string &type, rapidjson::Document &json);

    const std::string getPublicKey() const { return m_publicKey; }

    void onMessage(connection_hdl hdl, message_ptr msg);
    void onCloseHandler(connection_hdl hdl);

    bool addClient(const ClientInfo &info);
    ClientInfoPtr getClient(uint32_t clientId);

    void addAuthorizedConnections(const connection_hdl &connection, uint32_t clientId);

    // 给所有满足权限的客户端发送通知.
    void sendToAllAhtorizedClients(const char *payload, size_t lenPayload, uint32_t permissions = 0);

private:
    using MapIdToClient = std::map<uint32_t, ClientInfoPtr>;
    using MapConnections = std::map<connection_hdl, uint32_t, std::owner_less<connection_hdl>>;

    WsServer                    m_server;
    int                         m_port;

    mbedtls_pk_context          m_rsaKey;
    mbedtls_ctr_drbg_context    m_ctr_drbg;
    mbedtls_entropy_context     m_entropy;
    std::string                 m_publicKey;

    MapIMessageHandlers         m_mapHandlers;

    std::recursive_mutex        m_mutex;

    MapIdToClient               m_mapClients;
    MapConnections              m_authorizedConnections;

};

} // namespace WebSocket

#endif /* Server_hpp */
