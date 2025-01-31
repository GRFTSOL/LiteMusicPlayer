//
//  FontInfoEx.cpp
//  MusicPlayer
//
//  Created by henry_xiao on 2023/1/4.
//

#include "FontInfoEx.h"


FontInfoEx::FontInfoEx() {
    height = 13;
    weight = FW_NORMAL;
    italic = 0;
    isUnderline = false;
}

FontInfoEx::FontInfoEx(cstr_t faceNameLatin9, cstr_t faceNameOthers, int size, int weight, int italic, bool isUnderline) :
    nameLatin9(faceNameLatin9), nameOthers(faceNameOthers), height(size),
    weight(weight), italic(italic), isUnderline(isUnderline)
{
    if (isEmptyString(faceNameOthers)) {
        this->nameOthers = faceNameLatin9;
    }
}

bool FontInfoEx::isSame(const FontInfoEx &font) const {
    return font.nameLatin9 == nameLatin9
        && font.nameOthers == nameOthers
        && font.height == height
        && font.weight == weight
        && font.italic == italic
        && font.isUnderline == isUnderline;
}

void FontInfoEx::clear() {
    *this = FontInfoEx();
}

bool FontInfoEx::parse(cstr_t text) {
    // szValue format: Verdana, 13, thin, 1, 0, Tahoma
    // Latin font, height, weight(bold,normal,thin), italic, underline, Other font

    CCommaSeparatedValues csv;
    VecStrings fields;

    csv.split(text, fields);
    trimStr(fields);
    if (fields.size() != 6) {
        return false;
    }

    nameLatin9 = fields[0];
    height = stringToInt(fields[1].c_str(), -1);

    auto &strWeight = fields[2];
    if (strWeight.empty()) {
        weight = 13;
    } else {
        if (strcasecmp(strWeight.c_str(), "bold") == 0) {
            weight = FW_BOLD;
        } else if (strcasecmp(strWeight.c_str(), "thin") == 0) {
            weight = FW_THIN;
        } else if (strcasecmp(strWeight.c_str(), "normal") == 0) {
            weight = FW_NORMAL;
        } else {
            weight = atoi(strWeight.c_str());
        }
    }

    italic = stringToInt(fields[3].c_str(), 0);
    // int nUnderLine = stringToInt(fields[4].c_str(), 0);
    nameOthers = fields[5];

    return true;
}

string FontInfoEx::toString() const {
    const char SZ_FEILD_SEP[] = ",";

    string text;
    text += nameLatin9; text += SZ_FEILD_SEP;
    text += itos(height); text += SZ_FEILD_SEP;
    text += itos(weight); text += SZ_FEILD_SEP;
    text += itos(italic); text += SZ_FEILD_SEP;
    text += "0,";
    text += nameOthers;

    return text;
}


#if UNIT_TEST

#include "utils/unittest.h"

TEST(FontInfoEx, parse) {
    cstr_t szValue = "Verdana, 13, thin, 1, 0, Tahoma";

    FontInfoEx font;
    ASSERT_TRUE(font.parse(szValue));
    ASSERT_TRUE(font.height == 13);
    ASSERT_TRUE(font.weight == FW_THIN);
    ASSERT_TRUE(font.italic == 1);
    ASSERT_TRUE(font.nameLatin9 == "Verdana");
    ASSERT_TRUE(font.nameOthers == "Tahoma");

    szValue = "Verdana,13,100,1,0,Tahoma";
    ASSERT_EQ(font.toString(), szValue);
}

#endif
