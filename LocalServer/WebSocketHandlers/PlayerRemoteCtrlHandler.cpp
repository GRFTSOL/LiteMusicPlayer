//
//  PlayerRemoteCtrlHandler.cpp
//  Mp3Player
//
//  Created by henry_xiao on 2023/1/27.
//

#include "PlayerRemoteCtrlHandler.hpp"
#include "Utils/Utils.h"
#include "MPlayer/Player.h"


LoopMode loopModeFromString(cstr_t loop);

namespace WebSocketHandlers {

PlayerRemoteCtrlHandler::PlayerRemoteCtrlHandler() {
    m_type = TYPE_PLAYER_REMOTE_CTRL;
}

/**
 * 弹出窗口询问是否接受远程连接的控制.
 */
void PlayerRemoteCtrlHandler::onMessage(Server *server, const websocketpp::connection_hdl &connection,
                                      uint32_t clientId, const rapidjson::Value &message)
{
    string cmd = getMemberString(message, "cmd");
    if (!cmd.empty()) {
        if (cmd == "play_pause") {
            g_player.playPause();
        } else if (cmd == "prev") {
            g_player.prev();
        } else if (cmd == "next") {
            g_player.next();
        } else if (cmd == "position") {
            int parameter = getMemberInt(message, "parameter", -1);
            if (parameter >= 0) {
                g_player.seekTo(parameter);
            }
        } else if (cmd == "settings.volume") {
            int parameter = getMemberInt(message, "parameter", -1);
            if (parameter >= 0 && parameter <= 100) {
                g_player.setVolume(parameter);
            }
        } else if (cmd == "settings.shuffle") {
            int parameter = getMemberInt(message, "parameter", -1);
            if (parameter != -1) {
                g_player.setShuffle(parameter != 0);
            }
        } else if (cmd == "settings.mute") {
            int parameter = getMemberInt(message, "parameter", -1);
            if (parameter != -1) {
                g_player.setMute(parameter != 0);
            }
        } else if (cmd == "settings.loop") {
            string parameter = getMemberString(message, "parameter");
            g_player.setLoop(loopModeFromString(parameter.c_str()));
        } else if (cmd == "play_list") {
            auto listIDs = getMemberIntArray(message, "parameter");
            auto playlist = g_player.getMediaLibrary()->getMediaByIDs(listIDs);
            g_player.setNowPlaying(playlist);
            g_player.play();
        } else if (cmd == "queue_playlist") {
            auto listIDs = getMemberIntArray(message, "parameter");
            auto tmp = g_player.getMediaLibrary()->getMediaByIDs(listIDs);
            auto playlist = g_player.getNowPlaying();
            playlist->insert(-1, tmp.get());
        }
    }

    sendResult(server, connection, clientId, RC_OK);
}

} // namespace WebSocketHandlers
