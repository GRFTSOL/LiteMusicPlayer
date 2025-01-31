//
//  InitConnectionHandler.hpp
//  MusicPlayer
//
//  Created by henry_xiao on 2023/1/25.
//

#ifndef InitConnectionHandler_hpp
#define InitConnectionHandler_hpp

#include "../WebSocket/Server.hpp"


namespace WebSocketHandlers {

using namespace WebSocket;

class InitConnectionHandler : public IMessageHandler {
public:
    InitConnectionHandler();

    virtual void onMessage(Server *server, const websocketpp::connection_hdl &connection,
                           uint32_t clientId, const rapidjson::Value &message) override;

};

} // namespace WebSocketHandlers

#endif /* InitConnectionHandler_hpp */
