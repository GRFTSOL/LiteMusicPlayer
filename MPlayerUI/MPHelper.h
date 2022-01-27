#pragma once

#include "PlayListFile.h"

void getDefaultPlaylistName(string &strPlaylistFile);

bool onSongOpenFileCmd(Window *pWndParent, bool bOpen);

bool onSongOpenDirCmd(Window *pWndParent, bool bOpen);

bool onCmdSongAddDirToMediaLib(Window *pWndParent);

bool onCmdSongAddFilesToMediaLib(Window *pWndParent);

void enumPlaylists(cstr_t szDir, int &nLevel, vector<string> &vFiles);

void enumPlaylistsFast(vector<string> &vFiles);

