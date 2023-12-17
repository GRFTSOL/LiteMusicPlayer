//
//  AacFileTags.cpp
//

#include <bitset>
#include "AacFileTags.hpp"
#include "ID3v2IF.h"
#include "LrcParser.h"
#include "../TinyJS/utils/BinaryStream.h"
#include "../TinyJS/utils/BinaryFileStream.h"


namespace MediaTags {


uint32_t getId3v2TagSize(FilePtr &fp) {
    uint8_t buf[10];

    auto bytes = fp.read(buf, sizeof(buf));
    if (bytes != sizeof(bytes)) {
        return 0;
    }

    if (memcmp(buf, "ID3", 3) == 0) {
        auto tagsize = (buf[6] << 21) | (buf[7] << 14) | (buf[8] << 7) | (buf[9] << 0);
        tagsize += 10;
        return tagsize;
    } else {
        return 0;
    }
}

bool lookForAacHeader(FilePtr &fp) {
    uint8_t buf[1024 * 16];
    int remain = 0;

    for (int i = 0; i < 32; i++) {
        auto len = fp.read(buf + remain, sizeof(buf) - remain);
        if (len <= 4) {
            return false;
        }

        auto p = buf, end = buf + len - 4;
        while (p <= end) {
            if ((p[0] == 0xff && (p[1] & 0xf6) == 0xf0) ||
                (p[0] == 'A' && p[1] == 'D' && p[2] == 'I' && p[3] == 'F')) {
                fp.seek(-(end - p) - 4, SEEK_CUR);
                return true;
            } else {
                p++;
            }
        }

        remain = int(end - p);
        memmove(buf, p, remain);
    }

    return false;
}


void parseAdts(FilePtr &fp, ExtendedMediaInfo &extendedInfoOut) {
    static int ADTS_SAMPLE_RATES[] = { 96000,88200,64000,48000,44100,32000,24000,22050,16000,12000,11025,8000,7350,0,0,0};

    size_t frameSizeTotal = 0;
    int frames = 0;
    InputBufferFile buf;

    buf.bind(fp.ptr(), 1024 * 16);

    while (true) {
        if (buf.size() > 7) {
            // check syncword
            auto p = buf.data();
            if (!(p[0] == 0xFF && (p[1] & 0xF6) == 0xF0)) {
                break;
            }

            if (frames == 0) {
                extendedInfoOut.sampleRate = ADTS_SAMPLE_RATES[(p[2] & 0x3c) >> 2];
            }

            int frameSize = ((p[3] & 0x3) << 11) | (p[4] << 3) | (p[5] >> 5);
            if (frameSize == 0) {
                break;
            }

            frameSizeTotal += frameSize;
            buf.forward(frameSize);
            frames++;
        } else if (!buf.fill()) {
            break;
        }
    }

    auto framesPerSec = extendedInfoOut.sampleRate / 1024.0f;
    float bytesPerFrame = frames ? frameSizeTotal / (frames * 1000.0) : 0;

    extendedInfoOut.bitRate = (int)(8. * bytesPerFrame * framesPerSec + 0.5);
    if (framesPerSec != 0) {
        extendedInfoOut.mediaLength = frames * 1000.0 / framesPerSec;
    } else {
        extendedInfoOut.mediaLength = 1;
    }

    extendedInfoOut.bitsPerSample = 16;
    extendedInfoOut.channels = 2;
}

string AacFileTags::getFileKind(const StringView &fileExt) {
    return "MPEG-2 Advanced Audio Coding (aac)";
}

int AacFileTags::getTags(cstr_t fileName, BasicMediaTags &tagsOut, ExtendedMediaInfo &extendedInfoOut) {
    FilePtr fp;
    if (!fp.open(fileName, "rb")) {
        return ERR_OPEN_FILE;
    }

    CID3v2IF id3v2(ED_SYSDEF);

    id3v2.open(fp.ptr(), false);
    id3v2.getTags(tagsOut);

    auto tagSize = getId3v2TagSize(fp);
    fp.seek(tagSize, SEEK_SET);
    if (lookForAacHeader(fp)) {
        parseAdts(fp, extendedInfoOut);
    }

    return ERR_OK;
}

VecStrings AacFileTags::enumLyrics(cstr_t fileName) {
    VecStrings lyricsUrls;

    FilePtr fp;
    if (!fp.open(fileName, "rb")) {
        return lyricsUrls;
    }

    CID3v2IF id3v2(ED_SYSDEF);
    int ret = id3v2.open(fp.ptr(), true);
    if (ret == ERR_OK) {
        id3v2.listLyrics(lyricsUrls);
    }

    // Put Sync lyrics at start
    VecStrings syncLyrics, unsyncLyrics;
    for (auto &str : lyricsUrls) {
        StringView url(str);
        if (url.startsWith(SZ_SONG_ID3V2_SYLT) || url.startsWith(SZ_SONG_ID3V2_LYRICS)) {
            syncLyrics.push_back(str);
        } else {
            unsyncLyrics.push_back(str);
        }
    }

    syncLyrics.insert(syncLyrics.end(), unsyncLyrics.begin(), unsyncLyrics.end());
    return syncLyrics;
}

int AacFileTags::getLyrics(cstr_t fileName, cstr_t lyricsUrl, bool isUseSpecifiedEncoding, CharEncodingType encodingSpecified, RawLyrics &rawLyricsOut) {
    FilePtr fp;
    if (!fp.open(fileName, "rb")) {
        return ERR_OPEN_FILE;
    }

    StringView url(lyricsUrl);

    if (!url.startsWith(SZ_SONG_ID3V2)) {
        assert(0 && "Not supported");
        return ERR_NOT_SUPPORT_FILE_FORMAT;
    }

    CID3v2IF id3v2(ED_SYSDEF);
    int ret = id3v2.open(fp.ptr(), true);
    if (ret != ERR_OK) {
        return ret;
    }

    if (url.startsWith(SZ_SONG_ID3V2_USLT)) {
        ID3v2UnsynchLyrics lyrics;
        ret = id3v2.getUnsyncLyrics(lyrics, lyricsUrl);
        if (ret != ERR_OK) {
            return ret;
        }

        return parseLyricsString(lyrics.m_strLyrics, false, rawLyricsOut);
    } else if (url.startsWith(SZ_SONG_ID3V2_SYLT)) {
        ID3v2SynchLyrics lyrics;
        ret = id3v2.getSyncLyrics(lyrics, lyricsUrl);
        if (ret != ERR_OK) {
            return ret;
        }

        LyricsLine *line = nullptr;
        for (LrcSyllable &syllable : lyrics.m_vSynLyrics) {
            if (syllable.bNewLine) {
                line = NULL;
                continue;
            }
            if (!line || lyrics.m_bAllSyllableIsNewLine) {
                rawLyricsOut.push_back(LyricsLine(syllable.nTime, -1));
                line = &rawLyricsOut.lines().back();
            }

            line->appendPiece(syllable.nTime, -1, syllable.strText);
        }

        LyrTagParser tagParser(rawLyricsOut.properties());
        tagParser.parseLrcAllTags(lyrics.m_strContentDesc);

        return ERR_OK;
    } else if (url.startsWith(SZ_SONG_ID3V2_LYRICS)) {
        ID3v2TextUserDefined lyrics;
        id3v2.getUserDefLyrics(lyrics);
        return parseLyricsString(lyrics.m_strValue.c_str(), false, rawLyricsOut);
    } else {
        assert(0 && "Not supported");
        return ERR_NOT_SUPPORT_FILE_FORMAT;
    }
}

}
