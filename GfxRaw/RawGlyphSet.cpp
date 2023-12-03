//
//  RawGlyphSet.cpp
//  Mp3Player
//
//  Created by henry_xiao on 2023/1/4.
//

#include "RawGlyphSet.hpp"


#define MEM_CLEAN_DURATION      (1000 * 60 * 2)

CRawGlyphSetMgr::CRawGlyphSetMgr() {
}

CRawGlyphSetMgr::~CRawGlyphSetMgr() {
    m_listSet.clear();
}

CRawGlyphSet *CRawGlyphSetMgr::getGlyphSet(const FontInfoEx &font) {
    for (CRawGlyphSet *pSet : m_listSet) {
        if (pSet->isSame(font)) {
            pSet->addRef();
            return pSet;
        }
    }

    CRawGlyphSet *pSet = new CRawGlyphSet;

    pSet->create(font);
    pSet->addRef();
    m_listSet.push_back(pSet);

    return pSet;
}

void CRawGlyphSetMgr::removeRawGlyphSet(CRawGlyphSet *pRawGlyphSet) {
    for (LIST_SET::iterator it = m_listSet.begin(); it != m_listSet.end(); ++it) {
        CRawGlyphSet *pSet = *it;
        if (pSet == pRawGlyphSet) {
            m_listSet.erase(it);
            return;
        }
    }
}

void CRawGlyphSetMgr::enableAntialias(bool bEnable) {
#ifdef _WIN32
    if (g_bAntialiasFontEnabled == bEnable) {
        return;
    }

    g_bAntialiasFontEnabled = bEnable;

    for (LIST_SET::iterator it = m_listSet.begin(); it != m_listSet.end(); ++it) {
        CRawGlyphSet *pSet = *it;
        pSet->clearGlyph();
        pSet->m_rawGlyphBuilder.destroy();
        pSet->m_rawGlyphBuilder.init(*pSet);
    }
#endif
}

CRawGlyphSetMgr g_rawGlyphSetMgr;


Glyph::Glyph() {
    leftOffset = topOffset = heightBitmap = marginOutlined = 0;
    widthBitmap = nWidth = 0;
    bitmap = nullptr;
    bitmapOutlined = nullptr;
    nLastUsedTime = getTickCount();
    freed = false;
}

Glyph::~Glyph() {
    if (bitmapOutlined) {
        delete[] bitmapOutlined;
    }
    if (bitmap) {
        delete[] bitmap;
    }
}

CRawGlyphSet::CRawGlyphSet() {
    OBJ_REFERENCE_INIT

    m_timeLastClean = getTickCount();
}

CRawGlyphSet::~CRawGlyphSet() {
    clearGlyph();

    g_rawGlyphSetMgr.removeRawGlyphSet(this);
}

bool CRawGlyphSet::create(const FontInfoEx &font) {
    m_font = font;

    m_rawGlyphBuilder.init(font);

    return true;
}

int CRawGlyphSet::getHeight() const {
    return m_rawGlyphBuilder.getHeight();
}

Glyph *CRawGlyphSet::getGlyph(string &ch) {
    Glyph *glyph = nullptr;
    auto now = getTickCount();

    if (m_mapGlyph.size() > 256 && now - m_timeLastClean >= MEM_CLEAN_DURATION) {
        // Clean unused glyph
        m_timeLastClean = now;
        for (auto it = m_mapGlyph.begin(); it != m_mapGlyph.end(); ++it) {
            auto glyph = (*it).second;
            if (now - glyph->nLastUsedTime >= MEM_CLEAN_DURATION) {
                if (glyph->bitmap) {
                    delete[] glyph->bitmap;
                    glyph->bitmap = nullptr;
                    glyph->freed = true;
                    if (glyph->bitmapOutlined) {
                        delete[] glyph->bitmapOutlined;
                        glyph->bitmapOutlined = nullptr;
                    }
                }
            }
        }
    }

    auto it = m_mapGlyph.find(ch);
    if (it == m_mapGlyph.end()) {
        glyph = m_rawGlyphBuilder.buildGlyph(ch);
        assert(glyph);
        if (!glyph) {
            return nullptr;
        }
        m_mapGlyph[ch] = glyph;
    } else {
        glyph = (*it).second;
        if (glyph->freed && glyph->bitmap == nullptr) {
            // It must has been freed, renew one.
            Glyph *temp = m_rawGlyphBuilder.buildGlyph(ch);
            assert(temp);
            if (!temp) {
                return nullptr;
            }
            glyph->bitmap = temp->bitmap;
            glyph->freed = false;
            temp->bitmap = nullptr;
            delete temp;
        }
    }

    glyph->nLastUsedTime = now;

    return glyph;
}

void CRawGlyphSet::clearGlyph() {
    for (MAP_GLYPH::iterator it = m_mapGlyph.begin(); it != m_mapGlyph.end(); ++it) {
        Glyph *p = (*it).second;
        delete p;
    }
    m_mapGlyph.clear();
}
