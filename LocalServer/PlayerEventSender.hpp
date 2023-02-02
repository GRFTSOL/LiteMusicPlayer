//
//  PlayerEventSender.hpp
//  Mp3Player
//
//  Created by henry_xiao on 2023/1/29.
//

#ifndef PlayerEventSender_hpp
#define PlayerEventSender_hpp

#include "../MPlayerUI/MPEventsDispatcher.h"
#include "WebSocket/Server.hpp"


class PlayerEventSender : public IEventHandler {
public:
    PlayerEventSender(WebSocket::Server *server);

    virtual void onEvent(const IEvent *pEvent);

protected:
    WebSocket::Server           *m_server;

};

using PlayerEventSenderPtr = std::shared_ptr<PlayerEventSender>;

#endif /* PlayerEventSender_hpp */
