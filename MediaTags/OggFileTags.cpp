//
//  OggTag.cpp
//  Taglib
//
//  Created by henry_xiao on 2023/8/28.
//

#include <bitset>
#include "OggFileTags.hpp"
#include "ID3/ID3v1.h"
#include "ID3v2IF.h"
#include "XiphComment.hpp"
#include "LrcParser.h"
#include "../TinyJS/utils/BinaryStream.h"
#include "../TinyJS/utils/BinaryFileStream.h"


namespace MediaTags {

class OggTag {
public:
    int open(cstr_t fileName, MediaDataType needMediaDataTypes);

protected:
    struct PageHeader {
        uint64_t absoluteGranularPosition = 0;
        uint32_t streamSerialNumber = 0;
        uint32_t pageSequenceNumber = 0;
        uint32_t crc32 = 0;

        bool firstPacketContinued = false;
        bool firstPageOfStream = false;
        bool lastPageOfStream = false;

        uint8_t countSizes = 0;
        uint8_t sizes[256];
    };

    void parsePageHeader(BinaryFileInputStream &fs, PageHeader &headerOut) throw(BinaryStreamOutOfRange);
    void parseStreamInfo(const StringView &data, uint64_t countGranular, size_t sizeMedia);

protected:
    friend class OggFileTags;

    FilePtr             _fp;

    XiphComment         _xiphComment;
    ExtendedMediaInfo   _mediaInfo;
    uint32_t            _mediaLength;
    uint32_t            _oggBeginPos = -1;

};

int OggTag::open(cstr_t fileName, MediaDataType needMediaDataTypes) {
    _fp.open(fileName, "rb");
    if (!_fp) {
        return ERR_OPEN_FILE;
    }

    BinaryFileInputStream fs(_fp);
    if (!fs.isOK()) {
        return ERR_OPEN_FILE;
    }

    int packetIdx = -1;
    const int IDX_READ_TILL = 2;
    string packets[IDX_READ_TILL];

    try {
        // Refer: https://www.xiph.org/ogg/doc/oggstream.html
        // Refer: https://www.xiph.org/ogg/doc/framing.html
        static StringView OggS("OggS");
        fs.find(OggS, 1024 * 1024 * 1); // 在 1 MB 的范围内查找 OggS 标记.
        _oggBeginPos = (uint32_t)fs.offset();
        uint64_t startGranularPosition = 0;

        while (packetIdx < IDX_READ_TILL) {
            PageHeader header;
            parsePageHeader(fs, header);
            if (packetIdx == -1) {
                startGranularPosition = header.absoluteGranularPosition;
            }

            if (!header.firstPacketContinued) {
                packetIdx++;
                if (packetIdx >= IDX_READ_TILL) {
                    break;
                }
            } else if (packetIdx == -1) {
                assert(0);
                packetIdx = 0;
            }

            for (int i = 0; i < header.countSizes; i++) {
                char buf[1024];
                auto size = header.sizes[i];
                fs.readBuf(buf, size);
                packets[packetIdx].append(buf, size);

                if (size < 255 && i < header.countSizes - 1) {
                    packetIdx++;
                    if (packetIdx >= IDX_READ_TILL) {
                        break;
                    }
                }
            }
        }

        if (packetIdx >= 2) {
            static StringView headerComments("\x03vorbis"), headerSetup("\x01vorbis");

            if (StringView(packets[0]).startsWith(headerSetup) && (needMediaDataTypes & MDT_EXT_INFO)) {
                fs.setOffsetToEnd();
                fs.rfind(OggS);
                PageHeader header;
                parsePageHeader(fs, header);
                auto countGranular = header.absoluteGranularPosition - startGranularPosition;
                auto size = fs.size() - _oggBeginPos;

                parseStreamInfo(StringView(packets[0]).substr(headerSetup.len), countGranular, size);
            }

            if (StringView(packets[1]).startsWith(headerComments)) {
                _xiphComment.parse(packets[1].substr(headerComments.len), needMediaDataTypes);
            }
        }
    } catch (std::exception &e) {
        ERR_LOG1("Failed to open flac file, error: %s", e.what());
    }

    return ERR_OK;
}

void OggTag::parsePageHeader(BinaryFileInputStream &fs, PageHeader &headerOut) throw(BinaryStreamOutOfRange) {
    string header = fs.readString(27);
    BinaryInputStream is(header);

    if (!is.readString(4).equal("OggS") || is.readUInt8() != 0) {
        throw std::exception();
    }
    const std::bitset<8> flags(is.readUInt8()); // 5

    headerOut.firstPacketContinued = flags.test(0);
    headerOut.firstPageOfStream = flags.test(1);
    headerOut.lastPageOfStream = flags.test(2);

    headerOut.absoluteGranularPosition = is.readUInt64(); // 6
    headerOut.streamSerialNumber = is.readUInt32(); // 14
    headerOut.pageSequenceNumber = is.readUInt32(); // 18

    headerOut.crc32 = is.readUInt32(); // 22

    headerOut.countSizes = is.readUInt8(); // 26
    fs.readBuf(headerOut.sizes, headerOut.countSizes);
}

void OggTag::parseStreamInfo(const StringView &data, uint64_t countGranular, size_t sizeMedia) {
    // Refer https://xiph.org/vorbis/doc/Vorbis_I_spec.html 4.2.2. Identification header
    BinaryInputStream is(data);

    uint32_t version = is.readUInt32();
    if (version != 0) {
        throw BinaryStreamOutOfRange(__LINE__);
    }

    _mediaInfo.channels = is.readUInt8();
    uint32_t sampleRate = is.readUInt32();
    /* uint32_t bitrateMaximum = */ is.readUInt32();
    uint32_t bitrateNominal = is.readUInt32();
    /* uint32_t bitrateMinimum = */ is.readUInt32();

    if (sampleRate > 0) {
        _mediaInfo.mediaLength = (uint32_t)(countGranular * 1000.0 / sampleRate + 0.5);
    }

    _mediaInfo.sampleRate = sampleRate;
    _mediaInfo.bitsPerSample = 16; // TODO...
    _mediaInfo.bitRate = bitrateNominal / 1000;
    if (_mediaInfo.bitRate == 0 && _mediaLength > 0) {
        _mediaInfo.bitRate = int(sizeMedia * 8 / (double)_mediaLength + 0.5);
    }
}


string OggFileTags::getFileKind(const StringView &fileExt) {
    return "Ogg Vorbis Audio (ogg)";
}

int OggFileTags::getTags(cstr_t fileName, BasicMediaTags &tagsOut, ExtendedMediaInfo &extendedInfoOut) {
    OggTag ogg;
    int ret = ogg.open(fileName, MDT_TAGS_EXT_INFO);
    if (ret != ERR_OK) {
        return ret;
    }

    ogg._xiphComment.getTags(tagsOut);
    extendedInfoOut = ogg._mediaInfo;
    return ERR_OK;
}

VecStrings OggFileTags::enumLyrics(cstr_t fileName) {
    OggTag ogg;
    int ret = ogg.open(fileName, MDT_LYRICS);
    if (ret == ERR_OK) {
        if (!ogg._xiphComment.getLyrics().empty()) {
            return { SZ_SONG_SOLE_EMBEDDED_LYRICS };
        }
    }

    return {};
}

int OggFileTags::getLyrics(cstr_t fileName, cstr_t lyricsUrl, bool isUseSpecifiedEncoding, CharEncodingType encodingSpecified, RawLyrics &rawLyricsOut) {
    if (StringView(lyricsUrl).equal(SZ_SONG_SOLE_EMBEDDED_LYRICS)) {
        OggTag ogg;
        int ret = ogg.open(fileName, MDT_LYRICS);
        if (ret != ERR_OK) {
            return ret;
        }

        auto &lyrics = ogg._xiphComment.getLyrics();
        return parseLyricsBinary(lyrics, isUseSpecifiedEncoding, encodingSpecified, rawLyricsOut);
    }

    return ERR_NOT_FOUND;
}

}
