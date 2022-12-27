#ifndef _MLFONT_INC_
#define _MLFONT_INC_

#pragma once

#ifndef DEFAULT_CHARSET
#define DEFAULT_CHARSET     1
#endif

typedef const struct __CTFont * CTFontRef;

class CMLFont : public CFontInfo {
public:
    CMLFont(void);
    virtual ~CMLFont(void);

public:
    virtual bool create(cstr_t szFaceNameLatin9, cstr_t szFaceNameOthers, int nSize, int nWeight, int nItalic, bool bUnderline = false);
    virtual bool create(const CFontInfo &font);

    virtual bool isValid() { return true; }

    virtual void destroy();

    // For MAC only.
    CTFontRef getHandle() const { return m_font; }

protected:
    CTFontRef                   m_font;

};

#endif // _MLFONT_INC_
