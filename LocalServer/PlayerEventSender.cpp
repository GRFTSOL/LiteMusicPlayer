//
//  PlayerEventSender.cpp
//  MusicPlayer
//
//  Created by henry_xiao on 2023/1/29.
//

#include "PlayerEventSender.hpp"
#include "Utils/rapidjson.h"


cstr_t loopModeToString(int loop) {
    switch (loop) {
        case MP_LOOP_OFF: return "off";
        case MP_LOOP_ALL: return "all";
        case MP_LOOP_TRACK: return "one";
        default: assert(0);
    }
    return "";
}

LoopMode loopModeFromString(cstr_t loop) {
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
        // ET_PLAYER_POS_UPDATE,
    };

    for (auto event : events) {
        registerHandler(MPlayerApp::getEventsDispatcher(), event);
    }
}

void writePlayerStatus(RapidjsonWriter &writer) {
    writer.Key("status");
    switch (g_player.getPlayerState()) {
        case PS_STOPPED: writer.String("stopped"); break;
        case PS_PAUSED: writer.String("paused"); break;
        case PS_PLAYING: writer.String("playing"); break;
        default: assert(0); writer.String("stopped"); break;
    }
}

void writePlayerPosition(RapidjsonWriter &writer) {
    writer.Key("position");
    writer.Int(g_player.getPlayPos());
}

void writeMedia(RapidjsonWriter &writer, Media *media) {
    writer.StartObject();
    writer.Key("id"); writer.Int(media->ID);
    writer.Key("artist"); writer.String(media->artist.c_str());
    writer.Key("album"); writer.String(media->album.c_str());
    writer.Key("title"); writer.String(media->title.c_str());
    writer.Key("year"); writer.Int(media->year);
    writer.Key("genre"); writer.String(media->genre.c_str());
    writer.Key("url"); writer.String(media->url.c_str());
    writer.Key("duration"); writer.Int(media->duration);
    writer.Key("fileSize"); writer.Int64(media->fileSize);
    writer.Key("timeAdded"); writer.Int64(media->timeAdded);
    writer.Key("timePlayed"); writer.Int64(media->timePlayed);
    writer.Key("lyricsFile"); writer.String(media->lyricsFile.c_str());
    writer.Key("rating"); writer.Int(media->rating);
    writer.Key("format"); writer.String(media->format.c_str());
    writer.Key("countPlayed"); writer.Int(media->countPlayed);
    writer.Key("bitRate"); writer.Int(media->bitRate);
    writer.Key("channels"); writer.Int(media->channels);
    writer.Key("bitsPerSample"); writer.Int(media->bitsPerSample);
    writer.Key("sampleRate"); writer.Int(media->sampleRate);
    writer.EndObject();
}

void writeCurPlaylist(RapidjsonWriter &writer) {
    writer.Key("cur_playlist");

    auto playlist = g_player.getNowPlaying();

    writer.StartArray();
    int count = playlist->getCount();
    for (int i = 0; i < count; i++) {
        auto media = playlist->getItem(i);
        if (media) {
            writeMedia(writer, media.get());
        }
    }
    writer.EndArray();
}

void writeCurMedia(RapidjsonWriter &writer) {
    writer.Key("cur_media");

    auto media = g_player.getCurrentMedia();
    if (media) {
        writeMedia(writer, media.get());
    } else {
        writer.Null();
    }
}

void writePlayerSettings(RapidjsonWriter &writer) {
    writer.Key("settings");

    writer.StartObject();
    writer.Key("shuffle"); writer.Bool(g_player.isShuffle());
    writer.Key("loop"); writer.String(loopModeToString(g_player.getLoop()));
    writer.Key("mute"); writer.Bool(g_player.isMute());
    writer.Key("volume"); writer.Int(g_player.getVolume());

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

void writeAllMediaLibrary(RapidjsonWriter &writer) {
    auto lib = g_player.getMediaLibrary();

    writer.StartObject();
    writer.Key("type");
    writer.String(TYPE_MEDIA_LIB_ALL);

    writer.Key("path_sep");
    writer.String(PATH_SEP_STR);

    writer.Key("list");
    writer.StartArray();

    auto playlist = lib->getAll();
    int count = playlist->getCount();
    for (int i = 0; i < count; i++) {
        auto media = playlist->getItem(i);
        if (media) {
            writeMedia(writer, media.get());
        }
    }

    writer.EndArray();
    writer.EndObject();
}

void PlayerEventSender::onEvent(const IEvent *event) {
    rapidjson::StringBuffer buf;
    RapidjsonWriter writer(buf);

    writer.StartObject();
    writer.Key("type");
    writer.String(TYPE_PLAYER_NOTIFICATION);

    switch (event->eventType) {
        case ET_PLAYER_STATUS_CHANGED: writePlayerStatus(writer); writePlayerPosition(writer); break;
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
