//
//  XiphComment.cpp
//  Taglib
//
//  Created by henry_xiao on 2023/8/29.
//

#include "XiphComment.hpp"


static StringView NAME_LYRICS("LYRICS");
static StringView NAME_COVERART("COVERART");

void XiphComment::parse(const StringView &data, MediaDataType needDataTypes) {
    // Refer: https://xiph.org/vorbis/doc/v-comment.html
    /*
    1) [vendor_length] = read an unsigned integer of 32 bits
    2) [vendor_string] = read a UTF-8 vector as [vendor_length] octets
    3) [user_comment_list_length] = read an unsigned integer of 32 bits
    4) iterate [user_comment_list_length] times {

         5) [length] = read an unsigned integer of 32 bits
         6) this iteration's user comment = read a UTF-8 vector as [length] octets

       }

    7) [framing_bit] = read a single bit as boolean
    8) if ( [framing_bit] unset or end of packet ) then ERROR
    9) done.
     */
    std::pair<StringView, string &> PROPS[] = {
        { "ARTIST", _tags.artist },
        { "ALBUM", _tags.album },
        { "TITLE", _tags.title },
        { "TRACKNUMBER", _tags.trackNo },
        { "DATE", _tags.year },
        { "GENRE", _tags.genre },
        { "DESCRIPTION", _tags.comments },
    };

    BinaryInputStream is(data);

    auto vendorLength = is.readUInt32();
    /*auto vendor = */is.readString(vendorLength);
    auto commentsCount = is.readUInt32();
    for (int i = 0; i < commentsCount; i++) {
        auto length = is.readUInt32();
        auto prop = is.readString(length);

        StringView name, value;
        if (prop.split('=', name, value)) {
            string tmpName = toUpper(name.toString().c_str());
            if (needDataTypes & MDT_TAGS) {
                for (auto &item : PROPS) {
                    if (item.first.equal(tmpName)) {
                        strJoin(item.second, ", ", value);
                        break;
                    }
                }
            } else if (NAME_LYRICS.equal(tmpName)) {
                if (needDataTypes & MDT_LYRICS) {
                    _lyrics.assign((char *)value.data, value.len);
                }
            } else if (NAME_COVERART.equal(tmpName)) {
                if (needDataTypes & MDT_PICTURES) {
                    // TODO...
                }
            }
        }

        if (needDataTypes & MDT_MODIFY) {
            _props.push_back(prop.toString());
        }
    }
}
