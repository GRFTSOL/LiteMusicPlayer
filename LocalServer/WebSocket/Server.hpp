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
#include "rapidjson/document.h"


namespace WebSocket {

using WsServer = websocketpp::server<websocketpp::config::asio>;

class IMessageHandler {
public:
    virtual const std::string &getMessageType();
    virtual void onMessage(const websocketpp::connection_hdl &connection, const rapidjson::Value &message);

};

using IMessageHandlerPtr = std::shared_ptr<IMessageHandler>;
using VecIMessageHandlers = std::vector<IMessageHandlerPtr>;
using MapIMessageHandlers = std::unordered_map<std::string, IMessageHandlerPtr>;


class Server {
public:
    using message_ptr = WsServer::message_ptr;

    Server(int port);

    void run();
    void registerMessageHandler(const IMessageHandlerPtr &handler);

    void send(const websocketpp::connection_hdl &connection, const std::string &payload);

    static void on_message(Server *s, websocketpp::connection_hdl hdl, message_ptr msg);

private:
    WsServer                    m_server;
    int                         m_port;

    MapIMessageHandlers         m_mapHandlers;

};

} // namespace WebSocket

#endif /* Server_hpp */
