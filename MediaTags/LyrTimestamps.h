#pragma once

#include "LyricsData.h"


namespace LyrTimestamps {

bool isTimestamps(cstr_t timestamps);

bool parse(cstr_t timestamps, RawLyrics &lyrLines);
string toString(const RawLyrics &lyrLines);

} // namespace LyrTimestamps
