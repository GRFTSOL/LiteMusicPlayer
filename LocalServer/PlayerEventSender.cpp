//
//  PlayerEventSender.cpp
//  Mp3Player
//
//  Created by henry_xiao on 2023/1/29.
//

#include "PlayerEventSender.hpp"
#include "Utils/rapidjson.h"


void writeAllMediaLibrary(RapidjsonWriter &writer) {
    CMPAutoPtr<IMediaLibrary> lib;
    g_Player.getMediaLibrary(&lib);

    writer.StartObject();
    writer.Key("type");
    writer.String(TYPE_MEDIA_LIB_ALL);

    writer.Key("list");
    writer.StartArray();

    auto playlist = lib->getAll();
    int count = playlist->getCount();
    for (int i = 0; i < count; i++) {
        CMPAutoPtr<IMedia> media;
        if (playlist->getItem(i, &media) == ERR_OK) {
            writer.StartObject();
            CXStr str;

            writer.Key("id"); writer.Int(media->getID());
            media->getArtist(&str); writer.Key("artist"); writer.String(str.c_str());
            media->getAlbum(&str); writer.Key("album"); writer.String(str.c_str());
            media->getTitle(&str); writer.Key("title"); writer.String(str.c_str());
            media->getSourceUrl(&str); writer.Key("file"); writer.String(str.c_str());
            writer.Key("duration"); writer.Int(media->getDuration());
            media->getAttribute(MA_GENRE, &str); writer.Key("genre"); writer.String(str.c_str());
            media->getAttribute(MA_YEAR, &str); writer.Key("year"); writer.String(str.c_str());

            writer.EndObject();
        }
    }

    writer.EndArray();
    writer.EndObject();
}

cstr_t loopModeToString(int loop) {
    switch (loop) {
        case MP_LOOP_OFF: return "off";
        case MP_LOOP_ALL: return "all";
        case MP_LOOP_TRACK: return "one";
        default: assert(0);
    }
    return "";
}

MP_LOOP_MODE loopModeFromString(cstr_t loop) {
    if (strcmp(loop, "off") == 0) return MP_LOOP_OFF;
    else if (strcmp(loop, "all") == 0) return MP_LOOP_ALL;
    else if (strcmp(loop, "one") == 0) return MP_LOOP_TRACK;
    return MP_LOOP_ALL;
}

PlayerEventSender::PlayerEventSender(WebSocket::Server *server) : m_server(server) {
    static EventType events[] = {
        ET_PLAYER_STATUS_CHANGED,
        ET_PLAYER_SEEK,
        ET_PLAYER_CUR_MEDIA_CHANGED,
        ET_PLAYER_CUR_MEDIA_INFO_CHANGED,
        ET_PLAYER_CUR_PLAYLIST_CHANGED,
        ET_PLAYER_SETTING_CHANGED,
        ET_PLAYER_POS_UPDATE,
    };

    for (auto event : events) {
        registerHandler(CMPlayerAppBase::getEventsDispatcher(), event);
    }
}

void writePlayerStatus(RapidjsonWriter &writer) {
    writer.Key("status");
    switch (g_Player.getPlayerState()) {
        case PS_STOPED: writer.String("stopped"); break;
        case PS_PAUSED: writer.String("paused"); break;
        case PS_PLAYING: writer.String("playing"); break;
        default: assert(0); writer.String("stopped"); break;
    }
}

void writePlayerPosition(RapidjsonWriter &writer) {
    writer.Key("position");
    writer.Int(g_Player.getPlayPos());
}

void writeCurPlaylist(RapidjsonWriter &writer) {
    writer.Key("cur_playlist");

    CMPAutoPtr<IPlaylist> playlist;
    g_Player.getCurrentPlaylist(&playlist);

    writer.StartArray();
    int count = playlist->getCount();
    for (int i = 0; i < count; i++) {
        CMPAutoPtr<IMedia> media;
        CXStr str;

        playlist->getItem(i, &media);
        writer.StartObject();
        writer.Key("id"); writer.Int(media->getID());
        media->getArtist(&str); writer.Key("artist"); writer.String(str.c_str());
        media->getAlbum(&str); writer.Key("album"); writer.String(str.c_str());
        media->getTitle(&str); writer.Key("title"); writer.String(str.c_str());
        media->getSourceUrl(&str); writer.Key("file"); writer.String(str.c_str());
        writer.Key("duration"); writer.Int(media->getDuration());
        writer.EndObject();
    }
    writer.EndArray();
}

void writeCurMedia(RapidjsonWriter &writer) {
    writer.Key("cur_media");

    writer.StartObject();
    writer.Key("id"); writer.Int(g_Player.getMediaID());
    writer.Key("artist"); writer.String(g_Player.getArtist());
    writer.Key("album"); writer.String(g_Player.getAlbum());
    writer.Key("title"); writer.String(g_Player.getTitle());
    writer.Key("file"); writer.String(g_Player.getSrcMedia());
    writer.Key("duration"); writer.Int(g_Player.getMediaLength());
    writer.EndObject();
}

void writePlayerSettings(RapidjsonWriter &writer) {
    writer.Key("settings");

    writer.StartObject();
    writer.Key("shuffle"); writer.Bool(g_Player.isShuffle());
    writer.Key("loop"); writer.String(loopModeToString(g_Player.getLoop()));
    writer.Key("mute"); writer.Bool(g_Player.isMute());
    writer.Key("volume"); writer.Int(g_Player.getVolume());

    writer.EndObject();
}

void writeAllPlayerStates(RapidjsonWriter &writer) {
    writer.StartObject();
    writer.Key("type");
    writer.String(TYPE_PLAYER_STATES);

    writePlayerStatus(writer);
    writePlayerPosition(writer);
    writeCurPlaylist(writer);
    writeCurMedia(writer);
    writePlayerSettings(writer);

    writer.EndObject();
}

void PlayerEventSender::onEvent(const IEvent *event) {
    rapidjson::StringBuffer buf;
    RapidjsonWriter writer(buf);

    writer.StartObject();
    writer.Key("type");
    writer.String(TYPE_PLAYER_NOTIFICATION);

    switch (event->eventType) {
        case ET_PLAYER_STATUS_CHANGED: writePlayerStatus(writer); break;
        case ET_PLAYER_SEEK:
        case ET_PLAYER_POS_UPDATE: writePlayerPosition(writer); break;
        case ET_PLAYER_CUR_PLAYLIST_CHANGED: writeCurPlaylist(writer); break;
        case ET_PLAYER_CUR_MEDIA_INFO_CHANGED:
        case ET_PLAYER_CUR_MEDIA_CHANGED: writeCurMedia(writer); break;
        case ET_PLAYER_SETTING_CHANGED: {
            writer.Key("settings");

            writer.StartObject();
            CEventPlayerSettingChanged *settingEvt = (CEventPlayerSettingChanged *)event;
            if (settingEvt->settingType == IMPEvent::MPS_SHUFFLE) {
                writer.Key("shuffle"); writer.Bool(settingEvt->value);
            } else if (settingEvt->settingType == IMPEvent::MPS_LOOP) {
                writer.Key("loop"); writer.String(loopModeToString(settingEvt->value));
            } else if (settingEvt->settingType == IMPEvent::MPS_MUTE) {
                writer.Key("mute"); writer.Bool(settingEvt->value);
            } else if (settingEvt->settingType == IMPEvent::MPS_VOLUME) {
                writer.Key("volume"); writer.Int(settingEvt->value);
            } else {
                assert(0);
                return;
            }
            writer.EndObject();
            break;
        }
        default:
            return;
    }

    writer.EndObject();

    m_server->sendToAllAhtorizedClients(buf.GetString(), buf.GetSize());
}
