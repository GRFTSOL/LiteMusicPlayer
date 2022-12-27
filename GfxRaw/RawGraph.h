#ifndef GfxRaw_RawGraph_h
#define GfxRaw_RawGraph_h

#pragma once

#include "../ImageLib/RawImageData.h"


#ifdef _WIN32
enum PixPosition {
    PIX_B                       = 0,
    PIX_G                       = 1,
    PIX_R                       = 2,
    PIX_A                       = 3, // Alpha channel
    PIX_SIZE                    = 4
};
#else // #ifdef _WIN32
enum PixPosition {
    PIX_R                       = 0,
    PIX_G                       = 1,
    PIX_B                       = 2,
    PIX_A                       = 3, // Alpha channel
    PIX_SIZE                    = 4
};
#endif // #ifdef _WIN32


#ifdef _WIN32
#include "win32/RawGraphData.h"
#endif

#ifdef _LINUX_GTK2
#include "gtk2/RawGraphData.h"
#endif

#ifdef _MAC_OS
#include "mac/RawGraphData.h"
#endif


class CRawBmpFont;

//
// CRawGraph is a 32 bit depth Graphics.
//
// Its graph buffer can be accessed directly.
//
class CRawGraph : public CRawGraphData {
public:
    CRawGraph();
    virtual ~CRawGraph();

    class CClipBoxAutoRecovery {
    public:
        CClipBoxAutoRecovery(CRawGraph *canvas) {
            m_canvas = canvas;
            canvas->getClipBoundBox(m_rcClipOrg);
        }
        ~CClipBoxAutoRecovery() {
            if (m_canvas) {
                m_canvas->resetClipBoundBox(m_rcClipOrg);
            }
        }

        void recover() {
            if (m_canvas) {
                m_canvas->resetClipBoundBox(m_rcClipOrg);
                m_canvas = nullptr;
            }
        }

    protected:
        CRawGraph                   *m_canvas;
        CRect                       m_rcClipOrg;

    };

    class COpacityBlendAutoRecovery {
    public:
        COpacityBlendAutoRecovery(CRawGraph *canvas, int nOpacityPainting) {
            if (nOpacityPainting != 255) {
                m_canvas = canvas;
                m_nOpacityPainting = canvas->getOpacityPainting();

                nOpacityPainting = m_nOpacityPainting * nOpacityPainting / 255;
                m_canvas->setOpacityPainting(nOpacityPainting);
            } else {
                m_canvas = nullptr;
            }
        }
        ~COpacityBlendAutoRecovery() {
            if (m_canvas) {
                m_canvas->setOpacityPainting(m_nOpacityPainting);
            }
        }

        void recover() {
            if (m_canvas) {
                m_canvas->setOpacityPainting(m_nOpacityPainting);
                m_canvas = nullptr;
            }
        }

    protected:
        CRawGraph                   *m_canvas;
        int                         m_nOpacityPainting;

    };

    virtual bool create(int cx, int cy, WindowHandleHolder *windowHandle, int nBitCount = 32);

    virtual void line(int x1, int y1, int x2, int y2);

    virtual void rectangle(int x, int y, int width, int height);

    virtual void setPen(CRawPen &pen);

    virtual void getClipBoundBox(CRect &rc);
    virtual void setClipBoundBox(const CRect &rc);
    virtual void resetClipBoundBox(const CRect &rc);
    virtual void clearClipBoundBox();

    virtual bool textOut(int x, int y, cstr_t szText, size_t nLen);
    virtual bool drawTextClip(cstr_t szText, size_t nLen, const CRect &rcPos, int xLeftClipOffset = 0);

    virtual bool textOutOutlined(int x, int y, cstr_t szText, size_t nLen, const CColor &clrText, const CColor &clrBorder);
    virtual bool drawTextClipOutlined(cstr_t szText, size_t nLen, const CRect &rcPos, const CColor &clrText, const CColor &clrBorder, int xLeftClipOffset = 0);
    virtual bool drawTextOutlined(cstr_t szText, size_t nLen, const CRect &rcPos, const CColor &clrText, const CColor &clrBorder, uint32_t uFormat = DT_SINGLELINE | DT_LEFT);

    virtual bool drawText(cstr_t szText, size_t nLen, const CRect &rcPos, uint32_t uFormat = DT_SINGLELINE | DT_LEFT);

    virtual bool getTextExtentPoint32(cstr_t szText, size_t nLen, CSize *pSize);


    virtual void setTextColor(const CColor &color) {
        m_clrText = color;
    }

public:
    void setFont(CRawBmpFont *font);

    void resetAlphaChannel(uint8_t byAlpha, const CRect *lpRect);
    void multiplyAlpha(uint8_t byAlpha, const CRect *lpRect);

    void vertAlphaFadeOut(const CRect *lpRect, bool bTop);
    void vertFadeOut(const CRect *lpRect, const CColor &clrBg, bool bTop);

    void horzAlphaFadeOut(const CRect *lpRect, bool bLeft);
    void horzFadeOut(const CRect *lpRect, const CColor &clrBg, bool bLeft);

    void fillRectXOR(const CRect *lpRect, const CColor &clrFill);

    void fillRect(const CRect *lpRect, RawImageData *pImgMask, const CRect *rcMask, const CColor &clrFill, BlendPixMode bpm = BPM_COPY);
    void fillRect(const CRect *lpRect, const CColor &clrFill, BlendPixMode bpm = BPM_COPY);

    void fillRect(int x, int y, int nWidth, int nHeight, const CColor &clrFill, BlendPixMode bpm = BPM_COPY) {
        CRect rc(x, y, x + nWidth, y + nHeight);

        fillRect(&rc, clrFill, bpm);
    }

    int getOpacityPainting() const { return m_nOpacityPainting; }
    int setOpacityPainting(int nOpacityPainting) { int nOrg = m_nOpacityPainting; m_nOpacityPainting = nOpacityPainting; return nOrg; }

    bool setEnableAlphaChannel(bool bEnable) {
        bool bOrg = m_bAlphaChannelEnabled;
        m_bAlphaChannelEnabled = (bEnable && m_imageData.bitCount == 32);
        return bOrg;
    }
    bool getEnableAlphaChannel() const { return m_bAlphaChannelEnabled; }

    //
    // The following APIs provide logical coordinate and real coordinate convert.
    //

    CPoint setOrigin(const CPoint &ptOrg);
    void getOrigin(CPoint &ptOrg) const { ptOrg = m_ptOrigin; }

    void getMappedClipBoundRect(CRect &rc) const { rc = m_rcClip; rc.left += m_ptOrigin.x; rc.top += m_ptOrigin.y; rc.right += m_ptOrigin.x; rc.bottom += m_ptOrigin.y; }

    void mapPoint(CPoint &pt) const { pt.x += m_ptOrigin.x; pt.y += m_ptOrigin.y; }
    void mapRect(CRect &rc) const { rc.left += m_ptOrigin.x; rc.top += m_ptOrigin.y; rc.right += m_ptOrigin.x; rc.bottom += m_ptOrigin.y; }

    template<class _int> void mapPoint(_int &x, _int &y) const { x += m_ptOrigin.x; y += m_ptOrigin.y; }
    template<class _int> _int mapX(_int x) { return x + m_ptOrigin.x; }
    template<class _int> _int mapY(_int y) { return y + m_ptOrigin.y; }

    template<class _int> _int revertMapX(_int x) { return x - m_ptOrigin.x; }
    template<class _int> _int revertMapY(_int y) { return y - m_ptOrigin.y; }

protected:
    void clipAndMapRect(const CRect *lpRect, int &x, int &y, int &xMax, int &yMax);

protected:
    CColor                      m_clrText;

    CRawPen                     m_pen;

    CRect                       m_rcClip;

    CRawBmpFont                 *m_rawBmpFont;

    // The Opacity of painting (lines, text, images) to graph
    int                         m_nOpacityPainting;

    // The origin of graph, default is 0, 0.
    CPoint                      m_ptOrigin;

    bool                        m_bAlphaChannelEnabled;

};

#include "RawImage.h"
// #include "RawBmpFont.h"


#endif // !defined(GfxRaw_RawGraph_h)
