#pragma once

bool savePlaylistAsM3u(IPlaylist *playList, cstr_t szFile);

bool loadPlaylist(IMPlayer *player, IPlaylist *playList, cstr_t szFile);

bool loadM3uPlaylist(IMPlayer *player, IPlaylist *playList, cstr_t szFile);
bool loadPlsPlaylist(IMPlayer *player, IPlaylist *playList, cstr_t szFile);
bool loadWplPlaylist(IMPlayer *player, IPlaylist *playList, cstr_t szFile);
