#pragma once

bool savePlaylistAsM3u(Playlist *playList, cstr_t szFile);

PlaylistPtr loadPlaylist(cstr_t szFile);

PlaylistPtr loadM3uPlaylist(cstr_t szFile);
PlaylistPtr loadPlsPlaylist(cstr_t szFile);
PlaylistPtr loadWplPlaylist(cstr_t szFile);
