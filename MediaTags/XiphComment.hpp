//
//  XiphComment.hpp
//  Taglib
//
//  Created by henry_xiao on 2023/8/29.
//

#ifndef XiphComment_hpp
#define XiphComment_hpp

#include "MediaTagTypes.hpp"


class XiphComment {
public:
    void parse(const StringView &data, MediaDataType needDataTypes);

    void getTags(BasicMediaTags &tagOut) const {
        tagOut = _tags;
    }

    void setTags(BasicMediaTags &tag);
    void setLyrics(const string &lyrics);
    const string &getLyrics() const { return _lyrics; }

    string compose();

protected:
    string                  _vendor;
    VecStrings              _props;

    BasicMediaTags          _tags;
    string                  _lyrics;
    VecStrings              _pics;

};

#endif /* XiphComment_hpp */
