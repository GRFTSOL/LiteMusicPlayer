//
//  MediaTagTypes.cpp
//  Taglib
//
//  Created by henry_xiao on 2023/1/1.
//

#include "MediaTagTypes.hpp"


#define MERGE_STR_FIELD(fieldName)                  \
    for (int i = 0; i < countTags; i++) {           \
        if (!tags[i].fieldName.empty()) {           \
            tagsOut.fieldName = tags[i].fieldName;  \
            break;                                  \
        }                                           \
    }

#define MERGE_INT_FIELD(fieldName)                  \
    for (int i = 0; i < countTags; i++) {           \
        if (tags[i].fieldName > 0) {               \
            tagsOut.fieldName = tags[i].fieldName;  \
            break;                                  \
        }                                           \
    }

void mergeMediaTags(BasicMediaTags *tags, int countTags, BasicMediaTags &tagsOut) {
    MERGE_STR_FIELD(artist);
    MERGE_STR_FIELD(title);
    MERGE_STR_FIELD(album);
    MERGE_STR_FIELD(year);
    MERGE_STR_FIELD(genre);
    MERGE_STR_FIELD(trackNo);
    MERGE_STR_FIELD(comments);
}
