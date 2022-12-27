#pragma once

#ifndef _CColor_INC_
#define _CColor_INC_

#include "../Utils/UtilsTypes.h"


#ifndef RGB
#define RGB(r,g,b)          ((COLORREF)(((uint8_t)(r)|((uint32_t)((uint8_t)(g))<<8))|(((uint32_t)(uint8_t)(b))<<16)))

#define GetRValue(rgb)      ((uint8_t)(rgb))
#define GetGValue(rgb)      ((uint8_t)(((rgb) >> 8) & 0xFF))
#define GetBValue(rgb)      ((uint8_t)((rgb)>>16))
#endif

class CColor {
public:
    CColor(COLORREF clr) {
        m_clr = clr;
        m_alpha = 0xFF;
    }

    CColor() {
        m_clr = 0;
        m_alpha = 0xFF;
    }

    void set(int r, int g, int b) { m_clr = RGB(r, g, b); }

    void set(COLORREF clr) { m_clr = clr; }
    void get(COLORREF &clr) const { clr = m_clr; }
    COLORREF get() const { return m_clr; }

    int r() const { return GetRValue(m_clr); }
    int g() const { return GetGValue(m_clr); }
    int b() const { return GetBValue(m_clr); }
    int a() const { return m_alpha; }

    void setAlpha(uint8_t alpha) { m_alpha = alpha; }
    uint8_t getAlpha() const { return m_alpha; }

    bool operator==(CColor right) { return m_clr == right.m_clr && m_alpha == right.m_alpha; }

protected:
    COLORREF                    m_clr;
    uint8_t                     m_alpha;

};

inline void preMultiplyColor(CColor &clr, uint8_t alpha) {
    clr.set(clr.r() * alpha / 255, clr.g() * alpha / 255, clr.b() * alpha / 255);
}

#endif // _CColor_INC_
