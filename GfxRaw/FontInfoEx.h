#ifndef _GfxRaw_FontInfoEx_h
#define _GfxRaw_FontInfoEx_h

#pragma once

#include "RawImage.h"
#include "Color.h"


#ifndef DEFAULT_CHARSET
#define DEFAULT_CHARSET     1
#endif

// Font slant
enum {
    FS_NORMAL                   = 0,
    FS_OBLIQUE,
    FS_ITALIC,
};

#ifndef WIN32

#define FW_THIN             100
#define FW_EXTRALIGHT       200
#define FW_LIGHT            300
#define FW_NORMAL           400
#define FW_MEDIUM           500
#define FW_SEMIBOLD         600
#define FW_BOLD             700
#define FW_EXTRABOLD        800
#define FW_HEAVY            900

#define FW_ULTRALIGHT       FW_EXTRALIGHT
#define FW_REGULAR          FW_NORMAL
#define FW_DEMIBOLD         FW_SEMIBOLD
#define FW_ULTRABOLD        FW_EXTRABOLD
#define FW_BLACK            FW_HEAVY

#endif

struct FontInfoEx {
    FontInfoEx();
    FontInfoEx(cstr_t faceNameLatin9, cstr_t faceNameOthers, int size, int weight, int italic, bool isUnderline = false);

    bool isSame(const FontInfoEx &font) const;
    bool isValid() const { return !nameLatin9.empty(); }

    cstr_t getName() const { return nameLatin9.c_str(); }
    cstr_t getNameOthers() const { return nameOthers.c_str(); }
    int getSize() const { return height; }
    int getWeight() const { return weight; }
    int getItalic() const { return italic; }
    int getHeight() const { return height; }

    void clear();

    bool parse(cstr_t text);
    string toString() const;

public:
    string                      nameLatin9, nameOthers;
    int                         height;
    int                         weight;
    uint8_t                     italic;
    bool                        isUnderline;

};

#endif // _GfxRaw_FontInfoEx_h
