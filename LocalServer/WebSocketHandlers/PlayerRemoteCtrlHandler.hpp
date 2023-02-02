//
//  PlayerRemoteCtrlHandler.hpp
//  Mp3Player
//
//  Created by henry_xiao on 2023/1/27.
//

#ifndef PlayerRemoteCtrlHandler_hpp
#define PlayerRemoteCtrlHandler_hpp

#include "../WebSocket/Server.hpp"


namespace WebSocketHandlers {

using namespace WebSocket;

class PlayerRemoteCtrlHandler : public IMessageHandler {
public:
    PlayerRemoteCtrlHandler();

    virtual void onMessage(Server *server, const websocketpp::connection_hdl &connection,
                           uint32_t clientId, const rapidjson::Value &message) override;

};

} // namespace WebSocketHandlers

#endif /* PlayerRemoteCtrlHandler_hpp */
