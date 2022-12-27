#include "../third-parties/Agg/include/agg_scanline_p.h"
#include "../third-parties/Agg/include/agg_renderer_scanline.h"
#include "../third-parties/Agg/include/agg_pixfmt_rgba.h"
#include "../third-parties/Agg/include/agg_pixfmt_rgb.h"
#include "../third-parties/Agg/include/agg_rasterizer_scanline_aa.h"
#include "../third-parties/Agg/include/agg_pixfmt_gray.h"
#include "../third-parties/Agg/include/agg_blur.h"
#include "GfxRaw.h"
#include "RawBmpFont.h"
#include "ImageBuffBlt.h"


#define PATTERN_COLOR_RANGE     100
#define MEM_CLEAN_DURATION      (1000 * 60 * 2)


bool g_bAntialiasFontEnabled = false;

inline uint8_t RGBToGray(int r, int g, int b) {
    // 5 = 四舍五入
    return (r * 3 + g * 6 + b + 5) / 10;
}

bool inline isCharComposed(uint32_t c) {
    // The Unicode range for Devanagari is U+0900 .. U+097F.
    return (c >= 0x590 && c <= 0x6ff) || (c >= 0x0E00 && c <= 0x0E7F) || (c >= 0x0900 && c <= 0x097F);
}

void inline readUtf8Char(const char *str, string &dst) {
    while (true) {
        unsigned char firstChar = (unsigned char)(str[0]);
        int n;

        if (firstChar < 0xc0)            n = 1;
        else if (firstChar < 0xe0)        n = 2;
        else if (firstChar < 0xf0)        n = 3;
        else                            n = 1;
        dst.append(str, n);

        // Is composed char?
        uint32_t value = 0;
        for (int i = 0; str[i] != 0; i++) {
            value <<= 8;
            value |= uint8_t(str[i]);
        }
        if (!isCharComposed(value)) {
            break;
        }
        str += n;
    }
}

// Right to left string iterator
class RtlStringIterator {
public:
    typedef string        CharType;

    RtlStringIterator(cstr_t szText) {
        m_szText = szText;
        m_nPos = 0;
        readCurChar();
    }

    void operator = (cstr_t szText) {
        m_szText = szText;
        m_nPos = 0;
        readCurChar();
    }

    // Is end of string
    bool isEOS() { return m_szText[m_nPos] == 0; }

    CharType &curChar() {
        return m_ch;
    }

    void operator ++() {
        m_nPos += m_ch.size();
        readCurChar();
    }

    int getPos() { return m_nPos; }
    void setPos(int nPos) {
        assert((uint32_t)nPos <= strlen(m_szText));
        m_nPos = nPos;
        readCurChar();
    }

private:
    void readCurChar() {
        m_ch.clear();

        readUtf8Char(m_szText + m_nPos, m_ch);
        //#else
        //        int n = m_nPos;
        //        m_ch += m_szText[n];
        //        if (isCharComposed(m_szText[n]))
        //        {
        //            n++;
        //            while (isCharComposed(m_szText[n]))
        //            {
        //                m_ch += m_szText[n];
        //                n++;
        //            }
        //        }
        //#endif
    }

protected:
    cstr_t                      m_szText;
    int                         m_nPos;
    CharType                    m_ch;

};


template<class pixfmt>
void copyBitmpDataFromGraphBuffer_t(Glyph *pGlyph, pixfmt &bufGraph) {
    int nHeightBitmap = bufGraph.height();

    // Determine the right of Bitmap.
    int nRightBitmap = min(pGlyph->nWidth + nHeightBitmap / 2, (int)bufGraph.width());
    uint8_t *pRowSrc = bufGraph.pix_ptr(nRightBitmap - 1, 0);
    for (; nRightBitmap > 0; nRightBitmap--) {
        uint8_t *p = pRowSrc;
        int y = 0;
        for (; y < nHeightBitmap; y++) {
            if (p[0] != 0 || p[1] != 0 || p[2] != 0) {
                break;
            }
            p += bufGraph.stride();
        }
        if (y != nHeightBitmap) {
            break;
        }
        pRowSrc -= pixfmt::pix_width;
    }
    assert(nRightBitmap <= min(pGlyph->nWidth + nHeightBitmap / 2, (int)bufGraph.width()));
    if (nRightBitmap == 0) {
        return;
    }

    // Determine the left of Bitmap.
    int nLeftBitmap = 0;
    pRowSrc = bufGraph.row_ptr(0);
    for (; nLeftBitmap < nRightBitmap; nLeftBitmap++) {
        uint8_t *p = pRowSrc;
        int y = 0;
        for (; y < nHeightBitmap; y++) {
            if (p[0] != 0 || p[1] != 0 || p[2] != 0) {
                break;
            }
            p += bufGraph.stride();
        }
        if (y != nHeightBitmap) {
            break;
        }
        pRowSrc += pixfmt::pix_width;
    }

    // Determine the top of Bitmap.
    int nTopBitmap = 0;
    pRowSrc = bufGraph.pix_ptr(nLeftBitmap, 0);
    for (; nTopBitmap < nHeightBitmap; nTopBitmap++) {
        uint8_t *p = pRowSrc;
        int x = nLeftBitmap;
        for (; x < nRightBitmap; x++) {
            if (p[0] != 0 || p[1] != 0 || p[2] != 0) {
                break;
            }
            p += pixfmt::pix_width;
        }
        if (x != nRightBitmap) {
            break;
        }
        pRowSrc += bufGraph.stride();
    }

    // Determine the bottom of Bitmap.
    int nBottomBitmap = nHeightBitmap - 1;
    pRowSrc = bufGraph.pix_ptr(nLeftBitmap, nBottomBitmap);
    for (; nBottomBitmap > nTopBitmap; nBottomBitmap--) {
        uint8_t *p = pRowSrc;
        int x = nLeftBitmap;
        for (; x < nRightBitmap; x++) {
            if (p[0] != 0 || p[1] != 0 || p[2] != 0) {
                break;
            }
            p += pixfmt::pix_width;
        }
        if (x != nRightBitmap) {
            break;
        }
        pRowSrc -= bufGraph.stride();
    }

    pGlyph->leftOffset = nLeftBitmap;
    pGlyph->topOffset = nTopBitmap;
    pGlyph->widthBitmap = nRightBitmap - nLeftBitmap + 1;
    pGlyph->heightBitmap = nBottomBitmap - nTopBitmap + 1;

    pGlyph->bitmap = new uint8_t[pGlyph->widthBytes() * pGlyph->heightBitmap];

    pRowSrc = bufGraph.pix_ptr(pGlyph->leftOffset, pGlyph->topOffset);
    uint8_t *pRowDst = pGlyph->bitmap;
    for (int y = 0; y < pGlyph->heightBitmap; y++) {
        uint8_t *pDst = pRowDst;
        uint8_t *pSrc = pRowSrc;
        for (int x = 0; x < pGlyph->widthBitmap; x++) {
            //             pDst[G_R] = 255;
            //             pDst[G_G] = 255;
            //             pDst[G_B] = 255;
            //             pDst[G_A] = 255;
#ifdef _CLEAR_TYPE
            pDst[G_R] = pSrc[PixPosition::PIX_R];
            pDst[G_G] = pSrc[PixPosition::PIX_G];
            pDst[G_B] = pSrc[PixPosition::PIX_B];
#endif
            pDst[G_A] = RGBToGray(pSrc[G_R], pSrc[G_G], pSrc[G_B]);
            pDst += G_PIX_SIZE;
            pSrc += pixfmt::pix_width;
        }
        pRowSrc += bufGraph.stride();
        pRowDst += pGlyph->widthBytes();
    }
}

void copyBitmpDataFromGraphBuffer(Glyph *pGlyph, agg::pixfmt_rgb24 &bufGraph) {
    copyBitmpDataFromGraphBuffer_t(pGlyph, bufGraph);
}

void copyBitmpDataFromGraphBuffer(Glyph *pGlyph, agg::pixfmt_rgba32 &bufGraph) {
    copyBitmpDataFromGraphBuffer_t(pGlyph, bufGraph);
}

static void copyAndOutlineBuffRGBA32(uint8_t *pOut, uint8_t *pIn, int nWidth, int nHeight, int nMargin) {
    uint8_t *pIRow, *pORow;
    int y, c;
    int nWidthBytesOut = (nWidth + nMargin * 2) * G_PIX_SIZE;
    int nWidthBytesIn = nWidth * G_PIX_SIZE;

    // copyToOutlined(pOut, pIn, nWidth, nHeight, m_marginOutlined / 2, 0);

    // outline
    pIRow = pIn;
    pORow = pOut + nMargin * nWidthBytesOut + nMargin * G_PIX_SIZE;
    for (y = 0; y < nHeight; y++) {
        uint8_t *pI = pIRow, *pO = pORow;

        for (int x = 0; x < nWidth; x++) {
            for (int i = 0; i < 4; i++, pI++, pO++) {
                c = *pI;
                if (c == 0) {
                    continue;
                }
                if (c > *(pO))                        *(pO) = c;
                if (c > *(pO + G_PIX_SIZE))            *(pO + G_PIX_SIZE) = c;
                if (c > *(pO - G_PIX_SIZE))            *(pO - G_PIX_SIZE) = c;
                if (c > *(pO + nWidthBytesOut))        *(pO + nWidthBytesOut) = c;
                if (c > *(pO - nWidthBytesOut))        *(pO - nWidthBytesOut) = c;
            }
        }
        pIRow += nWidthBytesIn;
        pORow += nWidthBytesOut;
    }
}

static void eraseBuffRGBA32(uint8_t *pOut, uint8_t *pIn, int nWidth, int nHeight, int nMargin) {
    uint8_t *pIRow, *pORow;
    int x, y;
    int nWidthBytesOut = (nWidth + nMargin * 2) * G_PIX_SIZE;
    int nWidthBytesIn = nWidth * G_PIX_SIZE;

    pIRow = pIn;
    pORow = pOut + nMargin * nWidthBytesOut + nMargin * G_PIX_SIZE;
    for (y = 0; y < nHeight; y++) {
        uint8_t *pI = pIRow, *pO = pORow;

        for (x = 0; x < nWidth; x++) {
            if (pI[G_A] == 0xFF) {
                pO[G_A] = pO[G_R] = pO[G_G] = pO[G_B] = 0;
            }
            pI += 4;
            pO += 4;
        }
        pIRow += nWidthBytesIn;
        pORow += nWidthBytesOut;
    }
}

static void copyAndShadowBuffRGBA32(uint8_t *pOut, uint8_t *pIn, int nWidth, int nHeight, int nAlphaShadow, int nRangeShadow) {
    // copyToOutlined(pOut, pIn, nWidth, nHeight, nRangeShadow, 0);
    copyAndOutlineBuffRGBA32(pOut, pIn, nWidth, nHeight, nRangeShadow);

    agg::rendering_buffer buff(pOut, nWidth + nRangeShadow * 2, nHeight + nRangeShadow * 2, (nWidth + nRangeShadow * 2) * G_PIX_SIZE);
    agg::pixfmt_rgba32 pixf(buff);

    agg::stack_blur_rgba32(pixf, nRangeShadow, nRangeShadow);

    eraseBuffRGBA32(pOut, pIn, nWidth, nHeight, nRangeShadow);

    /*    uint8_t        *p, *pOutlined;
    int            y, c;

    // Shawdow
    p = pIn;
    pOutlined = pOut+ nMargin * nWidthOutlined;
    for (y = 0; y < nHeight; y++)
    {
    for (int x = 0; x < nWidth - 1; x++)
    {
    c = p[x];
    if (c == 0)
    continue;
    if (c > p[x + 1])
    {
    // x --> shadow
    }

    if (c > p[x + nWidth])
    {
    // y --> shadow

    }
    }
    p += nWidth;
    pOutlined += nWidthOutlined;
    }*/
}


void createOutlinedGlyph(Glyph *glyph, int marginOutlined, CRawBmpFont::ShadowMode shadowMode) {
    assert(glyph->bitmapOutlined == nullptr);

    glyph->marginOutlined = marginOutlined;

    int bytesBitmap = (glyph->heightBitmap + marginOutlined) * glyph->widthBytesOutlined();
    glyph->bitmapOutlined = new uint8_t[bytesBitmap];
    if (!glyph->bitmapOutlined) {
        return;
    }

    memset(glyph->bitmapOutlined, 0, bytesBitmap);

    if (shadowMode == CRawBmpFont::SM_SHADOW) {
        copyAndShadowBuffRGBA32(glyph->bitmapOutlined, glyph->bitmap, glyph->widthBitmap,
            glyph->heightBitmap, 255, marginOutlined / 2);
    } else {
        // Outline glyph
        copyAndOutlineBuffRGBA32(glyph->bitmapOutlined, glyph->bitmap, glyph->widthBitmap, glyph->heightBitmap, marginOutlined / 2);

        eraseBuffRGBA32(glyph->bitmapOutlined, glyph->bitmap, glyph->widthBitmap, glyph->heightBitmap, marginOutlined / 2);
    }
}

class CRawGlyphSetMgr {
public:
    CRawGlyphSetMgr() {
    }
    virtual ~CRawGlyphSetMgr() {
        m_listSet.clear();
    }

    CRawGlyphSet *getGlyphSet(const CFontInfo &font) {
        for (LIST_SET::iterator it = m_listSet.begin(); it != m_listSet.end(); ++it) {
            CRawGlyphSet *pSet = *it;
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

    void removeRawGlyphSet(CRawGlyphSet *pRawGlyphSet) {
        for (LIST_SET::iterator it = m_listSet.begin(); it != m_listSet.end(); ++it) {
            CRawGlyphSet *pSet = *it;
            if (pSet == pRawGlyphSet) {
                m_listSet.erase(it);
                return;
            }
        }
    }

    void enableAntialias(bool bEnable) {
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

protected:
    typedef list<CRawGlyphSet*>    LIST_SET;

    LIST_SET                    m_listSet;

};

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

bool CRawGlyphSet::create(const CFontInfo &font) {
    CFontInfo::create(font);

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

void tileBlt(CRawImage *pImage, CRawImage *canvas, int xDest, int yDest, int nWidthDest, int nHeightDest) {
    if (pImage->m_cy == 0 || pImage->m_cx == 0) {
        return;
    }

    int x, y;

    //
    // 先将Y方向的绘画了
    // 1111 1111 1111 222
    // 1111 1111 1111 222
    // 3333 3333 3333 444
    for (y = yDest; y + pImage->m_cy < yDest + nHeightDest; y += pImage->m_cy) {
        // 1111
        for (x = xDest; x + pImage->m_cx <= xDest + nWidthDest; x += pImage->m_cx) {
            pImage->blt(canvas,
                x, y,
                pImage->m_cx, pImage->m_cy,
                pImage->m_x, pImage->m_y);
        }

        // 2222
        if (x < xDest + nWidthDest) {
            pImage->blt(canvas,
                x, y,
                xDest + nWidthDest - x, pImage->m_cy,
                pImage->m_x, pImage->m_y);
        }
    }

    if (y < yDest + nHeightDest) {
        int nLeftCy;

        nLeftCy = yDest + nHeightDest - y;

        // 3333
        for (x = xDest; x + pImage->m_cx <= xDest + nWidthDest; x += pImage->m_cx) {
            pImage->blt(canvas,
                x, y,
                pImage->m_cx, nLeftCy,
                pImage->m_x, pImage->m_y);
        }

        // 444
        if (x < xDest + nWidthDest) {
            pImage->blt(canvas,
                x, y,
                xDest + nWidthDest - x, nLeftCy,
                pImage->m_x, pImage->m_y);
        }
    }
}
//
// void blur_text(uint8_t threshold, uint8_t decay, uint8_t max_depth, agg::rendering_buffer &iSrc, agg::rendering_buffer &iDst, uint8_t bytes)
// {
//
//     long x,y,z,m;
//     uint8_t *pSrc, *pSrc2, *pSrc3, *pDst;
//     uint8_t step,n;
//     int pivot;
//
//     if (max_depth<1) max_depth = 1;
//
//     long nmin,nmax,xmin,xmax,ymin,ymax;
//     xmin = ymin = 0;
//     xmax = iSrc.width();
//     ymax = iSrc.height();
//
//     if (xmin==xmax || ymin==ymax) return;
//
//     nmin = xmin * bytes;
//     nmax = xmax * bytes;
//
//     // double dbScaler = 100.0f/(ymax-ymin)/bytes;
//
//     for (n=0; n<bytes; n++){
//         for (y=ymin+1;y<(ymax-1);y++)
//         {
//             pSrc  = iSrc.row_ptr(y);
//             pSrc2 = iSrc.row_ptr(y+1);
//             pSrc3 = iSrc.row_ptr(y-1);
//             pDst  = iDst.row_ptr(y);
//
//             //scan left to right
//             for (x=n+nmin /*,i=xmin*/; x<(nmax-1); x+=bytes /*,i++*/)
//             {
//                 z=x+bytes;
//                 pivot = pSrc[z]-threshold;
//                 //find upper corner
//                 if (pSrc[x]<pivot && pSrc2[z]<pivot && pSrc3[x]>=pivot){
//                     while (z<nmax && pSrc2[z]<pSrc[x+bytes] && pSrc[x+bytes]<=pSrc[z]){
//                         z+=bytes;
//                     }
//                     m = z-x;
//                     m = (decay>1) ? ((m/bytes)/decay+1) : m/bytes;
//                     if (m>max_depth) m = max_depth;
//                     step = (uint8_t)((pSrc[x+bytes]-pSrc[x])/(m+1));
//                     while (m-->1){
//                         pDst[x+m*bytes] = (uint8_t)(pDst[x]+(step*(m+1)));
//                     }
//                 }
//                 //find lower corner
//                 z=x+bytes;
//                 if (pSrc[x]<pivot && pSrc3[z]<pivot && pSrc2[x]>=pivot){
//                     while (z<nmax && pSrc3[z]<pSrc[x+bytes] && pSrc[x+bytes]<=pSrc[z]){
//                         z+=bytes;
//                     }
//                     m = z-x;
//                     m = (decay>1) ? ((m/bytes)/decay+1) : m/bytes;
//                     if (m>max_depth) m = max_depth;
//                     step = (uint8_t)((pSrc[x+bytes]-pSrc[x])/(m+1));
//                     while (m-->1){
//                         pDst[x+m*bytes] = (uint8_t)(pDst[x]+(step*(m+1)));
//                     }
//                 }
//             }
//             //scan right to left
//             for (x=nmax-1-n /*,i=(xmax-1)*/; x>0; x-=bytes /*,i--*/)
//             {
//                 z=x-bytes;
//                 pivot = pSrc[z]-threshold;
//                 //find upper corner
//                 if (pSrc[x]<pivot && pSrc2[z]<pivot && pSrc3[x]>=pivot){
//                     while (z>n && pSrc2[z]<pSrc[x-bytes] && pSrc[x-bytes]<=pSrc[z]){
//                         z-=bytes;
//                     }
//                     m = x-z;
//                     m = (decay>1) ? ((m/bytes)/decay+1) : m/bytes;
//                     if (m>max_depth) m = max_depth;
//                     step = (uint8_t)((pSrc[x-bytes]-pSrc[x])/(m+1));
//                     while (m-->1){
//                         pDst[x-m*bytes] = (uint8_t)(pDst[x]+(step*(m+1)));
//                     }
//                 }
//                 //find lower corner
//                 z=x-bytes;
//                 if (pSrc[x]<pivot && pSrc3[z]<pivot && pSrc2[x]>=pivot){
//                     while (z>n && pSrc3[z]<pSrc[x-bytes] && pSrc[x-bytes]<=pSrc[z]){
//                         z-=bytes;
//                     }
//                     m = x-z;
//                     m = (decay>1) ? ((m/bytes)/decay+1) : m/bytes;
//                     if (m>max_depth) m = max_depth;
//                     step = (uint8_t)((pSrc[x-bytes]-pSrc[x])/(m+1));
//                     while (m-->1){
//                         pDst[x-m*bytes] = (uint8_t)(pDst[x]+(step*(m+1)));
//                     }
//                 }
//             }
//         }
//     }
// }


CRawBmpFont::CRawBmpFont() {
    m_prawGlyphSet = nullptr;

    m_rcClip.setEmpty();

    m_overlayMode = OM_COLOR;
    m_imgPattern1 = nullptr;
    m_imgPattern2 = nullptr;
    m_nAlphaPattern1 = 255;
    m_nAlphaPattern2 = 255;

    m_shadowMode = SM_SHADOW;
}

CRawBmpFont::~CRawBmpFont() {
    destroy();
}

bool CRawBmpFont::create(cstr_t szFaceNameLatin9, cstr_t szFaceNameOthers, int nHeight, int nWeight, int nItalic, bool bUnderline) {
    CFontInfo font;

    font.create(szFaceNameLatin9, szFaceNameOthers, nHeight, nWeight, nItalic, bUnderline);

    return create(font);
}

bool CRawBmpFont::create(const CFontInfo &font) {
    if (m_prawGlyphSet) {
        if (m_prawGlyphSet->isSame(font)) {
            return true;
        }

        m_prawGlyphSet->release();
    }

    m_prawGlyphSet = g_rawGlyphSetMgr.getGlyphSet(font);
    m_marginOutlined = getGlyphHeight() / 8 * 2;
    if (m_marginOutlined <= 1) {
        m_marginOutlined = 2;
    }

    return m_prawGlyphSet != nullptr;
}

void CRawBmpFont::destroy() {
    if (m_prawGlyphSet) {
        m_prawGlyphSet->release();
        m_prawGlyphSet = nullptr;
    }
}


int CRawBmpFont::getHeight() const {
    if (m_prawGlyphSet) {
        return m_prawGlyphSet->getHeight();
    } else {
        return 0;
    }
}

bool CRawBmpFont::textOut(CRawGraph *canvas, int x, int y, const CColor &clrText, cstr_t szText, size_t nLen, bool bDrawAlphaChannel) {
    return drawText(canvas, x, y, canvas->width() - x, 0, clrText, szText, nLen, bDrawAlphaChannel);
}

inline void blend_raw_font_clr(uint8_t *dst, uint8_t src, uint8_t clr);

inline void blend_raw_font_clr(uint8_t *dst, uint8_t src, uint8_t clr) {
    dst[0] = (uint8_t)(((clr - dst[0]) * src + (dst[0] << 8)) >> 8);
    //    dst[0] = (uint8_t)(((clr[0] - dst[0]) * src[0] + (dst[0] << 8)) >> 8);
}

class raw_glyph_get_alpha_255 {
public:
    inline uint8_t operator()(uint8_t alpha) {
        return alpha;
    }

};

class raw_glyph_get_alpha {
public:
    inline uint8_t operator()(uint8_t alpha) {
        return uint8_t(alpha * nGlyphAlpha / 255);
    }

    int                         nGlyphAlpha;

};

class raw_glyph_blend_alpha {
public:
    static inline void blend(uint8_t *dst, uint8_t src) {
        dst[3] = BlendAlpha(dst[3], src);
    }
};

class raw_glyph_blend_alpha_none {
public:
    static inline void blend(uint8_t *dst, uint8_t src) {
    }
};

template<class _RawFontAlphaBlender, class _RawGlyphGetAlpha>
void drawGlyphRGBA32Buff(agg::rendering_buffer &graph, int xDst, int yDst, int xDstEnd, int yDstEnd,
    agg::rendering_buffer &bufGlyph, int xSrc, int ySrc, _RawGlyphGetAlpha &getGlyphAlpha, const CColor &clrGlyph) {
    uint8_t *pRowDst = graph.row_ptr(yDst) + xDst * 4;
    uint8_t *pRowSrc = bufGlyph.row_ptr(ySrc) + xSrc * 4;
    int x, y;
    uint8_t clr[3];

    clr[PixPosition::PIX_B] = clrGlyph.b();
    clr[PixPosition::PIX_G] = clrGlyph.g();
    clr[PixPosition::PIX_R] = clrGlyph.r();

    for (y = yDst; y < yDstEnd; y++) {
        uint8_t *pDst = pRowDst;
        uint8_t *pSrc = pRowSrc;
        for (x = xDst; x < xDstEnd; x++) {
            if (pSrc[3] != 0) {
                blend_raw_font_clr(pDst, getGlyphAlpha(pSrc[0]), clr[0]);
                blend_raw_font_clr(pDst + 1, getGlyphAlpha(pSrc[1]), clr[1]);
                blend_raw_font_clr(pDst + 2, getGlyphAlpha(pSrc[2]), clr[2]);
                /*                    pDst[0] = (uint8_t)(((clr[0] - pDst[0]) * pSrc[0] + (pDst[0] << 8)) >> 8);
                 pDst[1] = (uint8_t)(((clr[1] - pDst[1]) * pSrc[1] + (pDst[1] << 8)) >> 8);
                 pDst[2] = (uint8_t)(((clr[2] - pDst[2]) * pSrc[2] + (pDst[2] << 8)) >> 8);*/
                _RawFontAlphaBlender::blend(pDst, getGlyphAlpha(pSrc[3]));
            }
            pDst += 4;
            pSrc += G_PIX_SIZE;
        }
        pRowDst += graph.stride();
        pRowSrc += bufGlyph.stride();
    }
}

// graph is 32 bpp, glyph is 32 bpp
void drawGlyphRGBA32(agg::rendering_buffer &graph, int xDst, int yDst, CRect &rcClip,
    bool bDrawOutlined, Glyph *pGlyph, int nAlphaGlyph, const CColor &clrGlyph, bool bDrawAlphaChannel) {
    assert(G_PIX_SIZE == 4);

    uint8_t clr[3];

    clr[PixPosition::PIX_B] = clrGlyph.b();
    clr[PixPosition::PIX_G] = clrGlyph.g();
    clr[PixPosition::PIX_R] = clrGlyph.r();

    int xSrc = 0, ySrc = 0;

    xDst -= MARGIN_FONT;
    yDst -= MARGIN_FONT;

    xDst += pGlyph->leftOffset;
    yDst += pGlyph->topOffset;
    int xDstEnd = xDst + pGlyph->widthBitmap;
    int yDstEnd = yDst + pGlyph->heightBitmap;

    // Set outlined glyph parameters
    agg::rendering_buffer bufGlyph;
    if (bDrawOutlined) {
        bufGlyph.attach(pGlyph->bitmapOutlined, pGlyph->widthOutlined(), pGlyph->heightOutlined(), pGlyph->widthBytesOutlined());
        xDst -= pGlyph->marginOutlined / 2;
        yDst -= pGlyph->marginOutlined / 2;
        xDstEnd += pGlyph->marginOutlined / 2;
        yDstEnd += pGlyph->marginOutlined / 2;
    } else {
        bufGlyph.attach(pGlyph->bitmap, pGlyph->widthBitmap, pGlyph->heightBitmap, pGlyph->widthBytes());
    }

    // Set clip area
    if (xDstEnd > rcClip.right) {
        xDstEnd = rcClip.right;
    }
    if (yDstEnd > rcClip.bottom) {
        yDstEnd = rcClip.bottom;
    }
    if (xDst < rcClip.left) {
        xSrc = rcClip.left - xDst;
        xDst = rcClip.left;
    }
    if (yDst < rcClip.top) {
        ySrc = rcClip.top - yDst;
        yDst = rcClip.top;
    }

    // Fill with color
    if (nAlphaGlyph == 255) {
        raw_glyph_get_alpha_255 getGlyphAlpha;
        if (bDrawAlphaChannel) {
            drawGlyphRGBA32Buff<raw_glyph_blend_alpha>(graph, xDst, yDst, xDstEnd, yDstEnd, bufGlyph, xSrc, ySrc, getGlyphAlpha, clrGlyph);
        } else {
            drawGlyphRGBA32Buff<raw_glyph_blend_alpha_none>(graph, xDst, yDst, xDstEnd, yDstEnd, bufGlyph, xSrc, ySrc, getGlyphAlpha, clrGlyph);
        }
    } else {
        raw_glyph_get_alpha getGlyphAlpha;
        getGlyphAlpha.nGlyphAlpha = nAlphaGlyph;
        if (bDrawAlphaChannel) {
            drawGlyphRGBA32Buff<raw_glyph_blend_alpha>(graph, xDst, yDst, xDstEnd, yDstEnd, bufGlyph, xSrc, ySrc, getGlyphAlpha, clrGlyph);
        } else {
            drawGlyphRGBA32Buff<raw_glyph_blend_alpha_none>(graph, xDst, yDst, xDstEnd, yDstEnd, bufGlyph, xSrc, ySrc, getGlyphAlpha, clrGlyph);
        }
    }

    /*    uint8_t *pRowDst = graph.row_ptr(yDst) + xDst * 4;
    uint8_t *pRowSrc = bufGlyph.row_ptr(ySrc) + xSrc * 4;

    // Fill with color
    if (nAlphaGlyph == 255)
    {
        for (y = yDst; y < yDstEnd; y++)
        {
            uint8_t *pDst = pRowDst;
            uint8_t *pSrc = pRowSrc;
            for (x = xDst; x < xDstEnd; x++)
            {
                if (pSrc[3] != 0)
                {
                    blend_raw_font_clr(pDst, pSrc, clr);
                    blend_raw_font_clr(pDst + 1, pSrc + 1, clr + 1);
                    blend_raw_font_clr(pDst + 2, pSrc + 2, clr + 2);
//                    pDst[0] = (uint8_t)(((clr[0] - pDst[0]) * pSrc[0] + (pDst[0] << 8)) >> 8);
//                    pDst[1] = (uint8_t)(((clr[1] - pDst[1]) * pSrc[1] + (pDst[1] << 8)) >> 8);
//                    pDst[2] = (uint8_t)(((clr[2] - pDst[2]) * pSrc[2] + (pDst[2] << 8)) >> 8);
                    _RawFontAlphaBlender::blend(pDst, pSrc);
                }
                pDst += 4;
                pSrc += G_PIX_SIZE;
            }
            pRowDst += graph.stride();
            pRowSrc += bufGlyph.stride();
        }
    }
    else
    {
        for (y = yDst; y < yDstEnd; y++)
        {
            uint8_t *pDst = pRowDst;
            uint8_t *pSrc = pRowSrc;
            for (x = xDst; x < xDstEnd; x++)
            {
                if (pSrc[3] != 0)
                {
                    int        a;
                    for (int i = 0; i < 3; i++)
                    {
                        a = pSrc[i] * nAlphaGlyph / 255;
                        pDst[i] = (uint8_t)(((clr[i] - pDst[i]) * a + (pDst[i] << 8)) >> 8);
                    }
                    a = pSrc[G_A] * nAlphaGlyph / 255;
                    pDst[G_A] = (uint8_t)((a + pDst[G_A]) - ((a * pDst[G_A] + 0xFF) >> 8));
                }
                pDst += 4;
                pSrc += G_PIX_SIZE;
            }
            pRowDst += graph.stride();
            pRowSrc += bufGlyph.stride();
        }
    }
*/
}

inline void getDrawGlyphClipPos(int &xDst, int &yDst, int &xDstEnd, int &yDstEnd, int &xSrc, int &ySrc, Glyph *pGlyph, CRect &rcClip) {
    xSrc = 0; ySrc = 0;

    xDst += pGlyph->leftOffset;
    yDst += pGlyph->topOffset;
    xDstEnd = xDst + pGlyph->widthBitmap;
    yDstEnd = yDst + pGlyph->heightBitmap;

    // Set clip area
    if (xDstEnd > rcClip.right) {
        xDstEnd = rcClip.right;
    }
    if (yDstEnd > rcClip.bottom) {
        yDstEnd = rcClip.bottom;
    }
    if (xDst < rcClip.left) {
        xSrc = rcClip.left - xDst;
        xDst = rcClip.left;
    }
    if (yDst < rcClip.top) {
        ySrc = rcClip.top - yDst;
        yDst = rcClip.top;
    }
}

// graph is 32 bpp, glyph is 32 bpp
// pattern is 24 bpp.
void drawGlyphRGBA32(agg::rendering_buffer &graph, int xDst, int yDst, CRect &rcClip,
    Glyph *pGlyph, int nAlphaGlyph,
    CRawImage *pattern) {
    uint8_t *pPt, *pRowPt;
    RawImageData *imgPattern;
    int x, y;

    int xDstEnd, yDstEnd, xSrc, ySrc;

    xDst -= MARGIN_FONT;
    yDst -= MARGIN_FONT;

    getDrawGlyphClipPos(xDst, yDst, xDstEnd, yDstEnd, xSrc, ySrc, pGlyph, rcClip);

    imgPattern = pattern->getHandle();
    assert(imgPattern->bitCount == 24);

    int xPtStart = xDst % imgPattern->width;
    int yPtStart = 0;
    yPtStart += ySrc + pGlyph->topOffset;
    if (yPtStart >= imgPattern->height) {
        yPtStart = 0;
    }

    pRowPt = imgPattern->rowPtr(yPtStart);
    pRowPt += xPtStart * 3;

    agg::rendering_buffer bufGlyph(pGlyph->bitmap, pGlyph->widthBitmap, pGlyph->heightBitmap, pGlyph->widthBytes());

    uint8_t *pRowDst = graph.row_ptr(yDst) + xDst * 4;
    uint8_t *pRowSrc = bufGlyph.row_ptr(ySrc) + xSrc * 4;

    for (y = yDst; y < yDstEnd; y++) {
        uint8_t *pDst = pRowDst;
        uint8_t *pSrc = pRowSrc;
        pPt = pRowPt;
        int xPtStartNew = xPtStart;
        for (x = xDst; x < xDstEnd; x++) {
            if (pSrc[3] != 0) {
                int a;
                for (int i = 0; i < 3; i++) {
                    a = pSrc[i] * nAlphaGlyph / 255;
                    pDst[i] = (uint8_t)(((pPt[i] - pDst[i]) * a + (pDst[i] << 8)) >> 8);
                }
                a = pSrc[G_A] * nAlphaGlyph / 255;
                pDst[G_A] = (uint8_t)((a + pDst[G_A]) - ((a * pDst[G_A] + 0xFF) >> 8));
                //                 int        a = pSrc[0] * nAlphaGlyph / 255;
                //                 pDst[0] = (uint8_t)(((pPt[0] - pDst[0]) * a + (pDst[0] << 8)) >> 8);
                //                 pDst[1] = (uint8_t)(((pPt[1] - pDst[1]) * a + (pDst[1] << 8)) >> 8);
                //                 pDst[2] = (uint8_t)(((pPt[2] - pDst[2]) * a + (pDst[2] << 8)) >> 8);
                //                 pDst[3] = (uint8_t)((a + pDst[3]) - ((a * pDst[3] + 0xFF) >> 8));
            }
            pDst += 4;
            pSrc += G_PIX_SIZE;

            xPtStartNew++;
            if (xPtStartNew >= imgPattern->width) {
                xPtStartNew = 0;
                pPt = imgPattern->rowPtr(yPtStart);
            } else {
                pPt += 3;
            }
        }
        pRowDst += graph.stride();
        pRowSrc += bufGlyph.stride();
        yPtStart++;
        if (yPtStart >= imgPattern->height) {
            yPtStart = 0;
            pRowPt = imgPattern->rowPtr(0);
            pRowPt += xPtStart * 3;
        } else {
            pRowPt += imgPattern->stride;
        }
    }
}

// graph is 32 bpp, glyph is 32 bpp
// pattern is 24 bpp.
void drawGlyphRGBA32(agg::rendering_buffer &graph, int xDst, int yDst, CRect &rcClip,
    Glyph *pGlyph, int nAlphaGlyph,
    CRawImage *pattern, int nAlphaPattern,
    const CColor &clrPt, int nAlphaClrPt) {
    uint8_t *pPt, *pRowPt;
    RawImageData *imgPattern;
    int x, y;
    uint8_t pclrPt[3];

    pclrPt[PixPosition::PIX_B] = clrPt.b();
    pclrPt[PixPosition::PIX_G] = clrPt.g();
    pclrPt[PixPosition::PIX_B] = clrPt.r();

    int xDstEnd, yDstEnd, xSrc, ySrc;

    xDst -= MARGIN_FONT;
    yDst -= MARGIN_FONT;

    getDrawGlyphClipPos(xDst, yDst, xDstEnd, yDstEnd, xSrc, ySrc, pGlyph, rcClip);

    imgPattern = pattern->getHandle();
    assert(imgPattern->bitCount == 24);

    int xPtStart = xDst % imgPattern->width;
    int yPtStart = 0;
    yPtStart += ySrc + pGlyph->topOffset;
    if (yPtStart >= imgPattern->height) {
        yPtStart = 0;
    }

    pRowPt = imgPattern->rowPtr(yPtStart);
    pRowPt += xPtStart * 3;

    agg::rendering_buffer bufGlyph(pGlyph->bitmap, pGlyph->widthBitmap, pGlyph->heightBitmap, pGlyph->widthBytes());

    uint8_t *pRowDst = graph.row_ptr(yDst) + xDst * 4;
    uint8_t *pRowSrc = bufGlyph.row_ptr(ySrc) + xSrc * 4;

    for (y = yDst; y < yDstEnd; y++) {
        uint8_t *pDst = pRowDst;
        uint8_t *pSrc = pRowSrc;
        pPt = pRowPt;
        int xPtStartNew = xPtStart;
        for (x = xDst; x < xDstEnd; x++) {
            if (pSrc[3] != 0) {
                int a, c;
                for (int i = 0; i < 3; i++) {
                    a = pSrc[i] * nAlphaGlyph / 255;
                    c = (pPt[i] * nAlphaPattern + pclrPt[i] * (255 - nAlphaPattern)) / 255;
                    pDst[i] = (uint8_t)(((c - pDst[i]) * a + (pDst[i] << 8)) >> 8);
                }
                a = pSrc[3] * nAlphaGlyph / 255;
                pDst[3] = (uint8_t)((a + pDst[3]) - ((a * pDst[3] + 0xFF) >> 8));
                //
                //                 int    r, g, b;
                //                 b = (pPt[0] * nAlphaPattern + bClrPt) / 255;
                //                 g = (pPt[1] * nAlphaPattern + gClrPt) / 255;
                //                 r = (pPt[2] * nAlphaPattern + rClrPt) / 255;
                //                 pDst[0] = (uint8_t)(((b - pDst[0]) * a + (pDst[0] << 8)) >> 8);
                //                 pDst[1] = (uint8_t)(((g - pDst[1]) * a + (pDst[1] << 8)) >> 8);
                //                 pDst[2] = (uint8_t)(((r - pDst[2]) * a + (pDst[2] << 8)) >> 8);
                //                 pDst[3] = (uint8_t)((a + pDst[3]) - ((a * pDst[3] + 0xFF) >> 8));
            }
            pDst += 4;
            pSrc += G_PIX_SIZE;

            xPtStartNew++;
            if (xPtStartNew >= imgPattern->width) {
                xPtStartNew = 0;
                pPt = imgPattern->rowPtr(yPtStart);
            } else {
                pPt += 3;
            }
        }
        pRowDst += graph.stride();
        pRowSrc += bufGlyph.stride();
        yPtStart++;
        if (yPtStart >= imgPattern->height) {
            yPtStart = 0;
            pRowPt = imgPattern->rowPtr(0);
        } else {
            pRowPt += imgPattern->stride;
        }
    }
}

// graph is 32 bpp, glyph is 32 bpp
// pattern is 24 bpp.
void drawGlyphRGBA32(agg::rendering_buffer &graph, int xDst, int yDst, CRect &rcClip,
    Glyph *pGlyph, int nAlphaGlyph,
    CRawImage *pattern1, int nAlphaPt1,
    CRawImage *pattern2, int nAlphaPt2) {
    uint8_t *pPt1, *pRowPt1;
    uint8_t *pPt2, *pRowPt2;
    RawImageData *imgPattern1, *imgPattern2;
    int x, y;

    int xDstEnd, yDstEnd, xSrc, ySrc;

    xDst -= MARGIN_FONT;
    yDst -= MARGIN_FONT;

    getDrawGlyphClipPos(xDst, yDst, xDstEnd, yDstEnd, xSrc, ySrc, pGlyph, rcClip);

    // pattern 1
    imgPattern1 = pattern1->getHandle();
    assert(imgPattern1->bitCount == 24);

    int xPt1Start = xDst % imgPattern1->width;
    int yPt1Start = 0;
    yPt1Start += ySrc + pGlyph->topOffset;
    if (yPt1Start >= imgPattern1->height) {
        yPt1Start = 0;
    }

    pRowPt1 = imgPattern1->rowPtr(yPt1Start);
    pRowPt1 += xPt1Start * 3;

    // pattern 2
    imgPattern2 = pattern2->getHandle();
    assert(imgPattern2->bitCount == 24);

    int xPt2Start = xDst % imgPattern2->width;
    int yPt2Start = 0;
    yPt2Start += ySrc + pGlyph->topOffset;
    if (yPt2Start >= imgPattern2->height) {
        yPt2Start = 0;
    }

    pRowPt2 = imgPattern2->rowPtr(yPt2Start);
    pRowPt2 += xPt2Start * 3;

    agg::rendering_buffer bufGlyph(pGlyph->bitmap, pGlyph->widthBitmap, pGlyph->heightBitmap, pGlyph->widthBytes());

    uint8_t *pRowDst = graph.row_ptr(yDst) + xDst * 4;
    uint8_t *pRowSrc = bufGlyph.row_ptr(ySrc) + xSrc * 4;

    for (y = yDst; y < yDstEnd; y++) {
        uint8_t *pDst = pRowDst;
        uint8_t *pSrc = pRowSrc;
        pPt1 = pRowPt1;
        pPt2 = pRowPt2;
        int xPt1StartNew = xPt1Start;
        int xPt2StartNew = xPt2Start;
        for (x = xDst; x < xDstEnd; x++) {
            if (pSrc[3] != 0) {
                int a, c;
                for (int i = 0; i < 3; i++) {
                    a = pSrc[i] * nAlphaGlyph / 255;
                    c = (pPt1[i] * nAlphaPt1 + pPt2[i] * nAlphaPt2) / 255;
                    pDst[i] = (uint8_t)(((c - pDst[i]) * a + (pDst[i] << 8)) >> 8);
                }
                a = pSrc[G_A] * nAlphaGlyph / 255;
                pDst[G_A] = (uint8_t)((a + pDst[G_A]) - ((a * pDst[G_A] + 0xFF) >> 8));
                //                 int        a = pSrc[0] * nAlphaGlyph / 255;
                //                 int        r, g, b;
                //                 b = (pPt1[0] * nAlphaPt1 + pPt2[0] * nAlphaPt2) / 255;
                //                 g = (pPt1[1] * nAlphaPt1 + pPt2[1] * nAlphaPt2) / 255;
                //                 r = (pPt1[2] * nAlphaPt1 + pPt2[2] * nAlphaPt2) / 255;
                //                 pDst[0] = (uint8_t)(((b - pDst[0]) * a + (pDst[0] << 8)) >> 8);
                //                 pDst[1] = (uint8_t)(((g - pDst[1]) * a + (pDst[1] << 8)) >> 8);
                //                 pDst[2] = (uint8_t)(((r - pDst[2]) * a + (pDst[2] << 8)) >> 8);
                //                 pDst[3] = (uint8_t)((a + pDst[3]) - ((a * pDst[3] + 0xFF) >> 8));
            }
            pDst += 4;
            pSrc += G_PIX_SIZE;

            xPt1StartNew++;
            if (xPt1StartNew >= imgPattern1->width) {
                xPt1StartNew = 0;
                pPt1 = imgPattern1->rowPtr(yPt1Start);
            } else {
                pPt1 += 3;
            }

            xPt2StartNew++;
            if (xPt2StartNew >= imgPattern2->width) {
                xPt2StartNew = 0;
                pPt2 = imgPattern2->rowPtr(yPt2Start);
            } else {
                pPt2 += 3;
            }
        }
        pRowDst += graph.stride();
        pRowSrc += bufGlyph.stride();

        yPt1Start++;
        if (yPt1Start >= imgPattern1->height) {
            yPt1Start = 0;
            pRowPt1 = imgPattern1->rowPtr(0);
        } else {
            pRowPt1 += imgPattern1->stride;
        }

        yPt2Start++;
        if (yPt2Start >= imgPattern2->height) {
            yPt2Start = 0;
            pRowPt2 = imgPattern2->rowPtr(0);
        } else {
            pRowPt2 += imgPattern2->stride;
        }
    }
}

bool CRawBmpFont::drawTextEx(CRawGraph *canvas, const CRect &rcPos, const CColor &clrText, cstr_t szText, size_t nLen, uint32_t uFormat, bool bDrawAlphaChannel) {
    assert(m_prawGlyphSet);
    if (!m_prawGlyphSet) {
        return false;
    }

    if (!isFlagSet(uFormat, DT_SINGLELINE)) {
        // Multiple line
        VecStrings vLines;
        CRect rc = rcPos;

        splitToMultiLine(rcPos, szText, nLen, uFormat, vLines);

        if (isFlagSet(uFormat, DT_VCENTER)) {
            int height = (int)vLines.size() * (m_prawGlyphSet->getHeight() + 2) - 2;
            rc.top = (rcPos.top + rcPos.bottom - height) / 2;
        }

        uFormat &= ~DT_VCENTER;
        for (int i = 0; i < (int)vLines.size(); i++) {
            drawTextEx(canvas, rc, clrText, vLines[i].c_str(), (int)vLines[i].size(), uFormat | DT_SINGLELINE, bDrawAlphaChannel);
            rc.top += getHeight() + 2;
            if (rc.top >= rc.bottom) {
                break;
            }
        }

        return true;
    } else if (isFlagSet(uFormat, DT_END_ELLIPSIS)) {
        string str;
        uFormat &= ~DT_END_ELLIPSIS;

        if (shouldDrawTextEllipsis(szText, nLen, rcPos.right - rcPos.left, str)) {
            return drawTextEx(canvas, rcPos, clrText, str.c_str(), (int)str.size(), uFormat, bDrawAlphaChannel);
        }
    } else if (isFlagSet(uFormat, DT_PREFIX_TEXT)) {
        string str;
        int nXPrefix, nPrefixWidth;

        uFormat &= ~DT_PREFIX_TEXT;

        if (shouldDrawTextPrefix(szText, nLen, str, nXPrefix, nPrefixWidth)) {
            drawTextEx(canvas, rcPos, clrText, str.c_str(), (int)str.size(), uFormat, bDrawAlphaChannel);

            if (nPrefixWidth > 0) {
                int x, y, width, xLeftClipOffset; //, height = rcPos.bottom - rcPos.top;

                getDrawTextExPosition(str.c_str(), (int)str.size(), rcPos, uFormat, x, y, width, xLeftClipOffset);
                x -= 1;
                y -= 3;
                x += nXPrefix;
                y += m_prawGlyphSet->getHeight();
                if (x > xLeftClipOffset) {
                    CRect rc;
                    CRawGraph::CClipBoxAutoRecovery cbr(canvas);
                    canvas->setClipBoundBox(rcPos);
                    rc.setLTRB(x, y + 1, x + nPrefixWidth, y + 2);
                    canvas->fillRect(&rc, clrText, BPM_CHANNEL_RGB | BPM_OP_COPY);
                }
            }

            return true;
        }
    }

    int x, y, width, xLeftClipOffset; //, height = rcPos.bottom - rcPos.top;

    getDrawTextExPosition(szText, nLen, rcPos, uFormat, x, y, width, xLeftClipOffset);

    x -= 1;
    y -= 1;

    return drawText(canvas, x, y, width, xLeftClipOffset, clrText, szText, nLen, bDrawAlphaChannel);
}

bool CRawBmpFont::drawText(CRawGraph *canvas, int x, int y, int width, int xLeftClipOffset, const CColor &clrText, cstr_t szText, size_t nLen, bool bDrawAlphaChannel) {
    uint8_t textAlpha = canvas->getOpacityPainting();
    CRect rcClip;
    RawImageData * pGraphRaw = canvas->getRawBuff();
    agg::rendering_buffer bufGraph(pGraphRaw->buff, pGraphRaw->width, pGraphRaw->height, pGraphRaw->stride);

    canvas->getMappedClipBoundRect(rcClip);

    if (x + xLeftClipOffset > rcClip.right) {
        return true;
    }
    if (x + xLeftClipOffset - m_marginOutlined / 2 > rcClip.left) {
        rcClip.left = x + xLeftClipOffset - m_marginOutlined / 2;
    }

    if (y >= rcClip.bottom) {
        return true;
    }
    if (y + getGlyphHeight() <= rcClip.top) {
        return true;
    }

    if (x + width + m_marginOutlined / 2 < rcClip.right) {
        rcClip.right = x + width + m_marginOutlined / 2;
    }

    RtlStringIterator strIterator(szText);
    for (; strIterator.getPos() < nLen; ++strIterator) {
        Glyph *glyph = m_prawGlyphSet->getGlyph(strIterator.curChar());
        if (!glyph) {
            continue;
        }

        if (x >= rcClip.right) {
            return true;
        }

        if (x + glyph->nWidth <= rcClip.left) {
            x += glyph->nWidth;
            continue;
        }

        if (m_overlayMode == OM_PATTERN) {
            drawGlyphRGBA32(bufGraph, x, y, rcClip, glyph, textAlpha, m_imgPattern1);
        } else if (m_overlayMode == OM_COLOR) {
            drawGlyphRGBA32(bufGraph, x, y, rcClip, false, glyph, textAlpha, clrText, bDrawAlphaChannel);
        } else if (m_overlayMode == OM_DUAL_PATTERN) {
            drawGlyphRGBA32(bufGraph, x, y, rcClip, glyph, textAlpha, m_imgPattern1, m_nAlphaPattern1, m_imgPattern2, m_nAlphaPattern2);
        } else if (m_overlayMode == OM_PATTERN_COLOR) {
            drawGlyphRGBA32(bufGraph, x, y, rcClip, glyph, textAlpha, m_imgPattern1, m_nAlphaPattern1, m_clrPattern2, m_nAlphaPattern2);
        }

        x += glyph->nWidth;
    }

    return true;
}

bool CRawBmpFont::outlinedTextOut(CRawGraph *canvas, int x, int y, const CColor &clrText, const CColor &clrBorder, cstr_t szText, size_t nLen, bool bDrawAlphaChannel) {
    return outlinedDrawText(canvas, x, y, canvas->width() - x, 0, clrText, clrBorder, szText, nLen, bDrawAlphaChannel);
}


bool CRawBmpFont::outlinedDrawTextEx(CRawGraph *canvas, const CRect &rcPos, const CColor &clrText, const CColor &clrBorder, cstr_t szText, size_t nLen, uint32_t uFormat, bool bDrawAlphaChannel) {
    assert(m_prawGlyphSet);
    if (!m_prawGlyphSet) {
        return false;
    }

    if (isFlagSet(uFormat, DT_END_ELLIPSIS)) {
        string str;

        uFormat &= ~DT_END_ELLIPSIS;

        if (shouldDrawTextEllipsis(szText, nLen, rcPos.right - rcPos.left, str)) {
            return outlinedDrawTextEx(canvas, rcPos, clrText, clrBorder, str.c_str(), (int)str.size(), uFormat, bDrawAlphaChannel);
        }
    } else if (isFlagSet(uFormat, DT_PREFIX_TEXT)) {
        string str;
        int nXPrefix, nPrefixWidth;

        uFormat &= ~DT_PREFIX_TEXT;

        if (shouldDrawTextPrefix(szText, nLen, str, nXPrefix, nPrefixWidth)) {
            outlinedDrawTextEx(canvas, rcPos, clrText, clrBorder, str.c_str(), (int)str.size(), uFormat, bDrawAlphaChannel);

            if (nPrefixWidth > 0) {
                int x, y, width, xLeftClipOffset; //, height = rcPos.bottom - rcPos.top;

                getDrawTextExPosition(str.c_str(), (int)str.size(), rcPos, uFormat, x, y, width, xLeftClipOffset);
                x -= 1;
                y -= 3;
                x += nXPrefix;
                y += m_prawGlyphSet->getHeight();
                if (x > xLeftClipOffset) {
                    CRect rc;
                    CRawGraph::CClipBoxAutoRecovery cbr(canvas);
                    canvas->setClipBoundBox(rcPos);
                    rc.setLTRB(x, y + 1, x + nPrefixWidth, y + 2);
                    canvas->fillRect(&rc, clrText, BPM_CHANNEL_RGB | BPM_OP_COPY);
                }
            }

            return true;
        }
    }


    int x, y, width, xLeftClipOffset; //, height = rcPos.bottom - rcPos.top;

    getDrawTextExPosition(szText, nLen, rcPos, uFormat, x, y, width, xLeftClipOffset);

    x -= 1;
    y -= 1;

    return outlinedDrawText(canvas, x, y, width, xLeftClipOffset, clrText, clrBorder, szText, nLen, bDrawAlphaChannel);
}

bool CRawBmpFont::outlinedDrawText(CRawGraph *canvas, int x, int y, int width, int xLeftClipOffset, const CColor &clrText, const CColor &clrBorder, cstr_t szText, size_t nLen, bool bDrawAlphaChannel) {
    uint8_t textAlpha = canvas->getOpacityPainting();
    CRect rcClip;

    RawImageData *pGraphRaw = canvas->getRawBuff();
    agg::rendering_buffer bufGraph(pGraphRaw->buff, pGraphRaw->width, pGraphRaw->height, pGraphRaw->stride);

    canvas->getMappedClipBoundRect(rcClip);

    if (x + xLeftClipOffset > rcClip.right) {
        return true;
    }
    if (x + xLeftClipOffset - m_marginOutlined / 2 > rcClip.left) {
        rcClip.left = x + xLeftClipOffset - m_marginOutlined / 2;
    }

    if (y >= rcClip.bottom) {
        return true;
    }
    if (y + getGlyphHeight() + m_marginOutlined / 2 <= rcClip.top) {
        return true;
    }

    if (x + width + m_marginOutlined / 2 < rcClip.right) {
        rcClip.right = x + width + m_marginOutlined / 2;
    }

    RtlStringIterator strIterator(szText);
    for (; strIterator.getPos() < nLen; ++strIterator) {
        Glyph *glyph = m_prawGlyphSet->getGlyph(strIterator.curChar());
        if (!glyph) {
            continue;
        }

        if (x >= rcClip.right) {
            return true;
        }

        if (x + glyph->nWidth <= rcClip.left) {
            x += glyph->nWidth;
            continue;
        }

        if (!glyph->bitmapOutlined && glyph->bitmap) {
            createOutlinedGlyph(glyph, m_marginOutlined, m_shadowMode);
        }

        if (glyph->bitmapOutlined) {
            //
            // Draw outlined border
            //
            drawGlyphRGBA32(bufGraph, x, y, rcClip, true, glyph, textAlpha, clrBorder, bDrawAlphaChannel);
        }

        if (glyph->bitmap) {
            //
            // Draw inner text
            //
            if (m_overlayMode == OM_PATTERN) {
                drawGlyphRGBA32(bufGraph, x, y, rcClip, glyph, textAlpha, m_imgPattern1);
            } else if (m_overlayMode == OM_COLOR) {
                drawGlyphRGBA32(bufGraph, x, y, rcClip, false, glyph, textAlpha, clrText, bDrawAlphaChannel);
            } else if (m_overlayMode == OM_DUAL_PATTERN) {
                drawGlyphRGBA32(bufGraph, x, y, rcClip, glyph, textAlpha, m_imgPattern1, m_nAlphaPattern1, m_imgPattern2, m_nAlphaPattern2);
            } else if (m_overlayMode == OM_PATTERN_COLOR) {
                drawGlyphRGBA32(bufGraph, x, y, rcClip, glyph, textAlpha, m_imgPattern1, m_nAlphaPattern1, m_clrPattern2, m_nAlphaPattern2);
            }
        }

        x += glyph->nWidth;
    }
    return true;
}

void CRawBmpFont::setOverlayPattern(CRawImage *imgPattern) {
    assert(imgPattern);
    m_imgPattern1 = imgPattern;
    m_overlayMode = OM_PATTERN;
    m_imgPattern2 = nullptr;
}

void CRawBmpFont::setOverlayPattern(CRawImage *imgPattern1, CRawImage *imgPattern2, int nAlphaPattern1, int nAlphaPattern2) {
    assert(imgPattern1);
    assert(imgPattern2);
    m_imgPattern1 = imgPattern1;
    m_imgPattern2 = imgPattern2;
    m_nAlphaPattern1 = nAlphaPattern1;
    m_nAlphaPattern2 = nAlphaPattern2;
    m_overlayMode = OM_DUAL_PATTERN;
}

void CRawBmpFont::setOverlayPattern(CRawImage *imgPattern1, int nAlphaPattern1, const CColor &clrPattern2, int nAlphaPattern2) {
    assert(imgPattern1);
    m_imgPattern1 = imgPattern1;
    m_clrPattern2 = clrPattern2;
    m_nAlphaPattern1 = nAlphaPattern1;
    m_nAlphaPattern2 = nAlphaPattern2;
    m_overlayMode = OM_PATTERN_COLOR;
    m_imgPattern2 = nullptr;
}

void CRawBmpFont::useColorOverlay() {
    m_imgPattern1 = nullptr;
    m_imgPattern2 = nullptr;
    m_overlayMode = OM_COLOR;
}

bool isWordSplitChar(cstr_t szChar) {
#ifdef UNICODE
    if ((*szChar) > 255 || !(isalpha(*szChar) || IsDigit(*szChar) || (*szChar >= 127 && *szChar <= 255)
        || *szChar == '\'' || *szChar == '\"' || *szChar == '_' || *szChar == '-')) {
        return true;
    }
#else
    uint32_t c = (uint32_t)*szChar;

    if (!(isalpha(c) || isDigit(c) || (c >= 127 && c <= 255)
        || c == '\'' || c == '\"' || c == '_' || c == '-')) {
        return true;
    }
#endif
    return false;
}

void CRawBmpFont::splitToMultiLine(const CRect &rcPos, cstr_t szText, size_t nLen, uint32_t uFormat, VecStrings &vLines) {
    int w = 0;
    Glyph *glyph;
    int nWidthMax = rcPos.right - rcPos.left;

    RtlStringIterator strIterator(szText);
    string str;
    int nBegin = 0, nLastWordSplitPos = 0;
    for (; strIterator.getPos() < nLen; ++strIterator) {
        glyph = m_prawGlyphSet->getGlyph(strIterator.curChar());
        if (!glyph) {
            continue;
        }

        if (isWordSplitChar(glyph->ch.c_str())) {
            nLastWordSplitPos = strIterator.getPos();
        }

        if (glyph->ch[0] == '\r' || glyph->ch[0] == '\n') {
            w = nWidthMax + 1;
        }
        w += glyph->nWidth;
        if (w > nWidthMax) {
            int nEnd = strIterator.getPos();
            if (nBegin < nLastWordSplitPos) {
                nEnd = nLastWordSplitPos;
            } else if (nEnd <= nBegin) {
                nEnd++;
            }

            str.clear();
            str.append(szText + nBegin, szText + nEnd);
            vLines.push_back(str);

            while (szText[nEnd] == ' ' || szText[nEnd] == '\r' || szText[nEnd] == '\n') {
                nEnd++;
            }

            w = 0;
            nBegin = nEnd;
            strIterator.setPos(nEnd - 1); // The for ++strIterator, will seek to next.
        }
    }

    if (nBegin < nLen) {
        str.clear();
        str.append(szText + nBegin);
        vLines.push_back(str);
    }
}

bool CRawBmpFont::shouldDrawTextEllipsis(cstr_t szText, size_t nLen, int nWidthMax, string &strEllipsis) {
    int w = 0;
    Glyph *glyph;

    RtlStringIterator strIterator(szText);
    for (; strIterator.getPos() < nLen; ++strIterator) {
        glyph = m_prawGlyphSet->getGlyph(strIterator.curChar());
        if (!glyph) {
            continue;
        }

        w += glyph->nWidth;
        if (w > nWidthMax) {
            break;
        }
    }

    if (strIterator.getPos() >= nLen) {
        return false;
    }

    int wMax;
    nLen = strIterator.getPos();

    static string dotStr = ".";
    glyph = m_prawGlyphSet->getGlyph(dotStr);
    if (!glyph) {
        return false;
    }
    wMax = nWidthMax - glyph->nWidth * 3;

    w = 0;
    for (strIterator.setPos(0); strIterator.getPos() < nLen; ++strIterator) {
        glyph = m_prawGlyphSet->getGlyph(strIterator.curChar());
        if (!glyph) {
            continue;
        }

        w += glyph->nWidth;
        if (w > wMax) {
            break;
        }

        strEllipsis += glyph->ch;
    }
    strEllipsis += "...";

    return true;
}


bool CRawBmpFont::shouldDrawTextPrefix(cstr_t szText, size_t nLen, string &strPrfix, int &nXPrefix, int &nWidthPrefix) {
    Glyph *glyph;

    nXPrefix = 0;
    nWidthPrefix = 0;

    RtlStringIterator strIterator(szText);
    for (; strIterator.getPos() < nLen; ++strIterator) {
        glyph = m_prawGlyphSet->getGlyph(strIterator.curChar());
        if (!glyph) {
            continue;
        }

        if (glyph->ch[0] == '&') {
            break;
        }

        nXPrefix += glyph->nWidth;
    }
    if (strIterator.getPos() >= nLen) {
        return false;
    }

    strPrfix = szText;
    strPrfix.erase(strIterator.getPos(), 1);

    if (strIterator.getPos() < (int)strPrfix.size()) {
        RtlStringIterator strIteratorNew(strPrfix.c_str());
        strIteratorNew.setPos(strIterator.getPos());

        glyph = m_prawGlyphSet->getGlyph(strIteratorNew.curChar());
        if (glyph) {
            nWidthPrefix = glyph->nWidth;
        }
    }

    return true;
}


int CRawBmpFont::getTextWidth(cstr_t szText, size_t nLen) {
    int w = 0;

    RtlStringIterator strIterator(szText);
    for (; strIterator.getPos() < nLen; ++strIterator) {
        Glyph *glyph = m_prawGlyphSet->getGlyph(strIterator.curChar());
        if (!glyph) {
            continue;
        }

        w += glyph->nWidth;
    }

    return w;
}
