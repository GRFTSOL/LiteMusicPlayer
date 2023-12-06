#pragma once

#include "../Utils/Utils.h"


void getArtistTitleFromFileName(string &strArtist, string &strTitle, cstr_t szFile);

/**
* Format media title in format of "artist - title"
**/
string formatMediaTitle(cstr_t szArtist, cstr_t szTitle);
