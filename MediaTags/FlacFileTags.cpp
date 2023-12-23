//
//  FlacFileTags.cpp
//  Taglib
//
//  Created by henry_xiao on 2023/8/28.
//

#include "FlacFileTags.hpp"
#include "ID3v2IF.h"
#include "ID3/ID3v1.h"
#include "XiphComment.hpp"
#include "LrcParser.h"
#include "../TinyJS/utils/BinaryFileStream.h"
#include "../TinyJS/utils/BinaryStream.h"


namespace MediaTags {

enum FlacBlockType {
    FBT_STREAMINFO              = 0,
    FBT_PADDING                 = 1,
    FBT_APPLICATION             = 2,
    FBT_SEEKTABLE               = 3,
    FBT_VORBIS_COMMENT          = 4,
    FBT_CUESHEET                = 5,
    FBT_PICTURE                 = 6,
    FBT_INVALID                 = 127,
};

class FlacTag {
public:
    virtual ~FlacTag();

    int open(cstr_t fileName, MediaDataType needMediaDataTypes);

    void getTags(BasicMediaTags &tags);
    int setTags(BasicMediaTags &tags);

    void getExtMediaInfo(ExtendedMediaInfo &mediaInfoOut);

    const string &getLyrics() const { return _xiphComment.getLyrics(); }

    VecStrings &getPictures() { return _pictures; }

protected:
    enum TagType {
        TT_ID3V1                = 1,
        TT_ID3V2                = 1 << 1,
        TT_XIPH_COMMENT         = 1 << 2,
    };

    enum TagIndex {
        IDX_XIPH_COMMENT,
        IDX_ID3V2,
        IDX_ID3V1,
        IDX_COUNT,
    };

    struct MetaDataBlock {
        uint8_t                 blockType;
        string                  data;

        MetaDataBlock(uint8_t type) : blockType(type) { }
    };
    using VecMetaDataBlocks = std::vector<MetaDataBlock>;

    void parseStreamInfo(const StringView &data);
    void parsePicture(const StringView &data);

protected:
    FILE                *_fp = nullptr;
    uint8_t             _tagTypes = 0;

    XiphComment         _xiphComment;

    BasicMediaTags      _tags[IDX_COUNT];
    ExtendedMediaInfo   _mediaInfo;
    uint32_t            _flacBeginPos = -1;
    VecMetaDataBlocks   _blocks;

    VecStrings          _pictures;

};

FlacTag::~FlacTag() {
    if (_fp) {
        fclose(_fp);
    }
}

int FlacTag::open(cstr_t fileName, MediaDataType needMediaDataTypes) {
    _tagTypes = 0;
    _fp = fopenUtf8(fileName, (needMediaDataTypes & MDT_MODIFY) ? "r+b" : "rb");
    if (!_fp) {
        return ERR_OPEN_FILE;
    }

    size_t endPosOfId3v2 = 0;
    CID3v2IF id3v2(ED_UTF8);
    if (id3v2.open(_fp, false) == ERR_OK) {
        endPosOfId3v2 = id3v2.getHeaderEndPosition();
        if (id3v2.getTags(_tags[IDX_ID3V2]) == ERR_OK) {
            _tagTypes |= TT_ID3V2;
        }
    }

    CID3v1 id3v1;
    if (id3v1.getTag(_fp, _tags[IDX_ID3V1]) == ERR_OK) {
        _tagTypes |= TT_ID3V1;
    }

    BinaryFileInputStream fs(_fp);
    if (!fs.isOK()) {
        return ERR_OPEN_FILE;
    }

    try {
        // Refer: https://xiph.org/flac/format.html
        fs.setOffset((uint32_t)endPosOfId3v2);
        fs.find("fLaC", 1024 * 1024 * 1); // 在 1 MB 的范围内查找 fLaC 标记.
        _flacBeginPos = (uint32_t)fs.offset();
        fs.forward(4);

        bool isLastBlock = false;

        while (!isLastBlock) {
            /*
            <1>    Last-metadata-block flag: '1' if this block is the last metadata block before the audio blocks, '0' otherwise.
            <7>    BLOCK_TYPE
                0 : STREAMINFO
                1 : PADDING
                2 : APPLICATION
                3 : SEEKTABLE
                4 : VORBIS_COMMENT
                5 : CUESHEET
                6 : PICTURE
                7-126 : reserved
                127 : invalid, to avoid confusion with a frame sync code
            <24>    Length (in bytes) of metadata to follow (does not include the size of the METADATA_BLOCK_HEADER)*/

            uint8_t b0 = fs.readUInt8();
            uint8_t blocktype = b0 & ~0x80;
            uint32_t length = fs.readUint24BE();

            isLastBlock = b0 & 0x80;

            /*if (needWrite) {
                MetaDataBlock block;
                block.data.resize(length);
                fs.readBuf((uint8_t *)block.data.c_str(), block.data.size());
                _blocks.push_back(block);
            } else {
                fs.forward(length);
            }*/

            if (blocktype == FBT_VORBIS_COMMENT) {
                MetaDataBlock block(blocktype);
                block.data = fs.readString(length);
                _xiphComment.parse(block.data, needMediaDataTypes);
                _xiphComment.getTags(_tags[IDX_XIPH_COMMENT]);
                _tagTypes |= TT_XIPH_COMMENT;

                if (needMediaDataTypes & MDT_MODIFY) {
                    _blocks.push_back(block);
                }
            } else if (blocktype == FBT_STREAMINFO) {
                if (needMediaDataTypes & (MDT_MODIFY | MDT_EXT_INFO)) {
                    MetaDataBlock block(blocktype);
                    block.data = fs.readString(length);
                    if (needMediaDataTypes & MDT_EXT_INFO) { parseStreamInfo(block.data); }
                    if (needMediaDataTypes & MDT_MODIFY) { _blocks.push_back(block); }
                } else {
                    fs.forward(length);
                }
            } else if (blocktype == FBT_PICTURE) {
                if (needMediaDataTypes & (MDT_MODIFY | MDT_PICTURES)) {
                    MetaDataBlock block(blocktype);
                    block.data = fs.readString(length);
                    if (needMediaDataTypes & MDT_PICTURES) { parsePicture(block.data); }
                    if (needMediaDataTypes & MDT_MODIFY) { _blocks.push_back(block); }
                } else {
                    fs.forward(length);
                }
            } else {
                if (needMediaDataTypes & MDT_MODIFY) {
                    MetaDataBlock block(blocktype);
                    block.data = fs.readString(length);
                    _blocks.push_back(block);
                } else {
                    fs.forward(length);
                }
            }
        }

        if ((needMediaDataTypes & MDT_EXT_INFO)) {
            auto length = getFileLength(fileName);
            if (_tagTypes & TT_ID3V1) {
                length -= ID3_V1Lengths::ID3_V1_LEN;
            }
            _mediaInfo.bitRate = (int)((length - fs.offset()) * 8.0 / _mediaInfo.mediaLength + 0.5);
        }
    } catch (std::exception &e) {
        ERR_LOG1("Failed to open flac file, error: %s", e.what());
    }

    return ERR_OK;
}

void FlacTag::getTags(BasicMediaTags &tagsOut) {
    mergeMediaTags(_tags, CountOf(_tags), tagsOut);
}

int FlacTag::setTags(BasicMediaTags &tags) {

    return ERR_OK;
}

void FlacTag::getExtMediaInfo(ExtendedMediaInfo &mediaInfoOut) {
    mediaInfoOut = _mediaInfo;
}

void FlacTag::parseStreamInfo(const StringView &data) {
    BinaryInputStream is(data);

    /*
     <16>    The minimum block size (in samples) used in the stream.
     <16>    The maximum block size (in samples) used in the stream. (Minimum blocksize == maximum blocksize) implies a fixed-blocksize stream.
     <24>    The minimum frame size (in bytes) used in the stream. May be 0 to imply the value is not known.
     <24>    The maximum frame size (in bytes) used in the stream. May be 0 to imply the value is not known.
      */
    is.forward(2 + 2 + 3 + 3);

    /*
     <20>    Sample rate in Hz. Though 20 bits are available, the maximum sample rate is limited by the structure of frame headers to 655350Hz. Also, a value of 0 is invalid.
     <3>    (number of channels)-1. FLAC supports from 1 to 8 channels
     <5>    (bits per sample)-1. FLAC supports from 4 to 32 bits per sample.
     <36>    Total samples in stream. 'Samples' means inter-channel sample, i.e. one second of 44.1Khz audio will have 44100 samples regardless of the number of channels. A value of zero here means the number of total samples is unknown.
     <128>    MD5 signature of the unencoded audio data. This allows the decoder to determine if an error exists in the audio data even when the error does not result in an invalid bitstream.
     NOTES
     FLAC specifies a minimum block size of 16 and a maximum block size of 65535, meaning the bit patterns corresponding to the numbers 0-15 in the minimum blocksize and maximum blocksize fields are invalid.
     */
    uint64_t n = is.readUInt64BE();
    _mediaInfo.sampleRate = (n >> 44) & 0xFFFFF;
    _mediaInfo.channels = ((n >> 41) & 0x7) + 1;
    _mediaInfo.bitsPerSample = ((n >> 36) & 0x1f) + 1;
    uint64_t samples = n & 0xFFFFFFFFFLL;
    if (_mediaInfo.sampleRate > 0) {
        _mediaInfo.mediaLength = uint32_t((samples * 1000LL + _mediaInfo.sampleRate - 1) / _mediaInfo.sampleRate);
    }
}

void FlacTag::parsePicture(const StringView &data) {
    BinaryInputStream is(data);

    try {
        /* auto type = */is.readUInt32();
        auto length = is.readUInt32();
        /* auto mime = */is.readString(length);

        length = is.readUInt32();
        /* auto description = */is.readString(length);

        // width + height + depth + indexed-color
        is.forward(4 + 4 + 4 + 4);

        length = is.readUInt32();
        auto imageData = is.readString(length);

        _pictures.push_back(imageData.toString());
    } catch (std::exception &e) {
        ERR_LOG1("Failed to parse flac picture data: %s", e.what());
    }
}

string FlacFileTags::getFileKind(const StringView &fileExt) {
    return "Free Lossless Audio Codec (flac)";
}

int FlacFileTags::getTags(cstr_t fileName, BasicMediaTags &tagsOut, ExtendedMediaInfo &extendedInfoOut) {
    FlacTag tag;
    int ret = tag.open(fileName, MDT_TAGS_EXT_INFO);
    if (ret != ERR_OK) {
        return ret;
    }

    tag.getTags(tagsOut);
    tag.getExtMediaInfo(extendedInfoOut);
    return ERR_OK;
}

VecStrings FlacFileTags::enumLyrics(cstr_t fileName) {
    FlacTag tag;
    int ret = tag.open(fileName, MDT_TAGS_EXT_INFO);
    if (ret == ERR_OK) {
        string strLyrics = tag.getLyrics();
        if (!strLyrics.empty()) {
            return { SZ_SONG_SOLE_EMBEDDED_LYRICS };
        }
    }

    return {};
}

int FlacFileTags::getLyrics(cstr_t fileName, cstr_t lyricsUrl, bool isUseSpecifiedEncoding, CharEncodingType encodingSpecified, RawLyrics &rawLyricsOut) {
    if (!StringView(lyricsUrl).equal(SZ_SONG_SOLE_EMBEDDED_LYRICS)) {
        return ERR_NOT_FOUND;
    }

    FlacTag tag;
    int ret = tag.open(fileName, MDT_LYRICS);
    if (ret != ERR_OK) {
        return ret;
    }

    string strLyrics = tag.getLyrics();
    if (strLyrics.empty()) {
        return ERR_NOT_FOUND;
    }

    return parseLyricsBinary(strLyrics, isUseSpecifiedEncoding, encodingSpecified, rawLyricsOut);
}

int FlacFileTags::getPicture(cstr_t fileName, uint32_t index, string &imageDataOut) {
    FlacTag tag;
    int ret = tag.open(fileName, MDT_PICTURES);
    if (ret != ERR_OK) {
        return ret;
    }

    if (index >= tag.getPictures().size()) {
        return ERR_NOT_FOUND;
    }

    imageDataOut = tag.getPictures()[index];

    return ERR_OK;
}

void FlacFileTags::getPictures(cstr_t fileName, VecStrings &vImagesDataOut) {
    FlacTag tag;
    int ret = tag.open(fileName, MDT_PICTURES);
    if (ret != ERR_OK) {
        vImagesDataOut.clear();
        return;
    }

    vImagesDataOut = tag.getPictures();
}

} // namespace MediaTags
