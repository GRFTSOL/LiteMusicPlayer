/********************************************************************
    Created  :    2002/01/04    21:40
    FileName :    LyricShowObj.h
    Author   :    xhy

    Purpose  :    
*********************************************************************/

#pragma once

#include "../MediaTags/LyricsData.h"
#include "../LyricsLib/MLLib.h"


enum LyrAlignment {
    LA_LEFT                     = 0,
    LA_CENTER                   = 1,
    LA_RIGHT                    = 2,
    LA_MAX                      = 3,
};

enum DISPLAY_OPTIONS {
    DO_NORMAL                   = 0, // Normal, no fade out
    DO_FADEOUT_LOWCOLOR,             // Fade out to low color
    DO_FADEOUT_BG,                   // Fade out to background color
    DO_AUTO,                         // If there are space for more than 4 or 5 lines,
    // we users fade out low color, or uses fade out background.
    DO_MAX
};

enum OverlayBlendingMode {
    OBM_UNKNOWN,
    OBM_COLOR,                       // 1 pure color
    OBM_GRADIENT_COLOR,              // 3 colors
    OBM_PATTERN,                     // picture pattern
};

extern IdToString g_idsLyrDisplayOpt[];

cstr_t displayOptToStr(DISPLAY_OPTIONS displayOpt);

DISPLAY_OPTIONS displayOptFromStr(cstr_t szDisplayOpt);

#define AUTO_CAL_X          0xFFFFFF

bool profileGetLyricsFont(cstr_t szSectName, FontInfoEx &info);
void profileWriteLyricsFont(EventType etColorTheme, cstr_t szSectName, const FontInfoEx &info);


#define LS_AD_DRAW_INIT            CLyricShowObj::draw(canvas)


class CLyricShowObj : public CUIObject, public IEventHandler {
    UIOBJECT_CLASS_NAME_DECLARE(CUIObject)
public:
    enum LinePos {
        LP_ABOVE_CUR_LINE,
        LP_CUR_LINE,
        LP_BELOW_CUR_LINE,
    };

    enum VertLineAlign {
        VLA_TOP,
        VLA_CENTER,
        VLA_BOTTOM,
    };

public:
    CLyricShowObj();
    virtual ~CLyricShowObj();

    class CDispOptRecover {
    public:
        CDispOptRecover(CLyricShowObj *pObj) { m_pObj = pObj; m_dispOpt = pObj->m_LyricsDisplayOpt; }
        ~CDispOptRecover() { m_pObj->m_LyricsDisplayOpt = m_dispOpt; }

        DISPLAY_OPTIONS             m_dispOpt;
        CLyricShowObj               *m_pObj;

    };
    friend class CDispOptRecover;

    // TOB = Text Overlay blending
    // TCI = Text Color Index
    enum TOBColorIndex {
        TCI_START                   = 0,
        TCI_FILL                    = TCI_START,
        TCI_GRADIENT1,
        TCI_GRADIENT2,
        TCI_GRADIENT3,
        TCI_BORDER,
        TCI_COUNT,
        TCI_OBM                     = TCI_COUNT, // obm name
        TCI_PATTERN,
    };

    struct TextOverlayBlending {
        TextOverlayBlending() : obm(OBM_UNKNOWN) { }

        OverlayBlendingMode         obm;                // Overlay blending mode
        CColor                      clr[TCI_COUNT];     // Colors, Accessed by TOBColorIndex
        CRawImage                   imgPattern;
        string                      strPatternFile;
    };

public:
    virtual void onCreate() override;

    // IEventHandle overrider
    virtual void onEvent(const IEvent *pEvent) override;

    virtual bool onKeyDown(uint32_t nChar, uint32_t nFlags) override;

    void draw(CRawGraph *canvas) override;

    // 快速绘画
    // OUTPUT:
    //        rcUpdate    -    更新的矩形区域
    virtual void fastDraw(CRawGraph *canvas, CRect *prcUpdate = nullptr);

    virtual bool onLButtonUp(uint32_t nFlags, CPoint point) override;
    virtual bool onLButtonDown(uint32_t nFlags, CPoint point) override;
    virtual bool onMouseDrag(CPoint point) override;
    virtual void onMagnify(float magnification) override;

    virtual void onPlayTimeChangedUpdate();

    virtual void onLyricsChanged();

    virtual void onSaveLyrics() { };

    virtual void setAntialise(bool bAntialis);

    int getAlpha(LyricsLine *pLine);

    void setLyricData(CMLData *pMLData);

    bool isKaraoke() { return m_bKaraoke || m_pMLData->isKaraokeShowStyle(); }

    //
    // set setting values
    //
    bool setProperty(cstr_t szProperty, cstr_t szValue) override;

    CLyricsLines &getLyrics() { return m_lyrLines; }

protected:
    virtual bool onLyrDisplaySettings(cstr_t szProperty, cstr_t szValue);

    void updateBgImage();

    void onSize() override;

    virtual void onLyrDrawContextChanged();
    virtual int getAutoHeightLines() { return 0; }

    void onAdjustHue(float hue, float saturation, float luminance) override;

    void wrapLyricsLines();

    void autoHeightSkinAccordLyrics(int nLines = 1);
    bool autoWidthSkinAccordLyrics();
    void updateLyricDrawBufferBackground(CRawGraph *canvas, CRect &rc);

    void darkenLyricsBg(CRawGraph *canvas, CRect &rc);

    int getLyricRowAlignPos(CRawGraph *canvas, LyricsLine *pLyricRow);
    int getLyricRowTextWidth(CRawGraph *canvas, LyricsLine *pLyricRow);

    void fastDrawMediaInfo(CRawGraph *canvas, CRect *prcUpdate = nullptr);

    bool drawRow(CRawGraph *canvas, LyricsLine *pLyricRow, int x, int y, LinePos linePos = LP_ABOVE_CUR_LINE);
    void drawRow(CRawGraph *canvas, LyricsLine *pLyricRow, int x, int y, CColor &clrTxt, CColor &clrTxtBorder);

    void drawRowKaraoke(CRawGraph *canvas, LyricsLine *pLyricRow, int x, int y);

    void drawCurrentRow(CRawGraph *canvas, LyricsLine *pLyricRow, int x, int y);

    void fadeOutVertBorder(CRawGraph *canvas, int yDrawLyrStartPos, int yDrawLyrEndPos);

    void loadBgImageFolder();

    void loadNextBgImage();

    void onTimer(int nId) override;

    bool isOutlineLyrics() const { return m_bOutlineLyrics || m_img.isValid(); }

    int getFontHeight() const { return m_nFontHeight + (isOutlineLyrics() ? MARGIN_FONT * 2 : 0); }

    int getPatternFontHeight() const { return m_nFontHeight + MARGIN_FONT * 2; }

    int getLineHeight() const { return m_nFontHeight + (isOutlineLyrics() ? MARGIN_FONT * 2 : 0) + m_nLineSpacing; }

    inline CColor &getHighlightColor() { return m_tobHilight.clr[TCI_FILL]; }
    inline CColor &getLowlightColor() { return m_tobLowlight.clr[TCI_FILL]; }

    void setLyrAlign(cstr_t szValue);
    void setBgColor(cstr_t szValue);

    void loadBgImageSettings();

    void loadLyrOverlaySettings();

    void loadLOBHilightSettings();
    void loadLOBLowlightSettings();

    void loadAllSettings();

    void reverseLyricsForRightToLeftLanguage();

protected:
    friend class CLyricShowAgentObj;

    // 边界
    int                         m_nXMargin, m_nYMargin;

    //
    // 歌词字体
    //
    CRawBmpFont                 m_font;
    int                         m_nFontHeight;

    // hue of lyrics
    float                       m_hue;

    bool                        m_bOutlineLyrics;

    // basic colors
    CColor                      m_clrBg;
    bool                        m_bSetSkinBg;

    //
    // blending lyrics options:
    //
    TextOverlayBlending         m_tobHilight;
    TextOverlayBlending         m_tobLowlight;

    // true: NEVER load customized settings.
    bool                        m_bUseSkinStyle;


    //
    // 显示选项: KARAOKE, 淡出为背景、淡出为低调色
    //
    DISPLAY_OPTIONS             m_LyricsDisplayOpt;

    bool                        m_bKaraoke;

    //
    // Use back ground image?
    //
    bool                        m_bUseAlbumArtAsBg;
    bool                        m_bUseBgImg;

    CCurMediaAlbumArt           m_curAlbumArt;
    string                      m_strBgPicFolder;
    VecStrings                  m_vPicFiles;
    int                         m_nextPic;
    int                         m_nextPicInFolder;
    CSFImage                    m_img;
    bool                        m_bDarkenLyrBgOnImg;
    float                       m_nDarkenTopArea, m_nDarkenBottomArea;
    CColor                      m_clrDarken;

    // Horizontal lyrics alignment
    LyrAlignment                m_nAlignment;

    // 行间距
    int                         m_nLineSpacing;

    bool                        m_bEnableAutoResize;

    Cursor                      m_Cursor;

    //
    // Lyrics vertical align position
    //
    Cursor                      m_curAdjustVertAlign;
    bool                        m_bEnableAdjustVertAlign, m_bEnableAdjustVertAlignUser;

    // Vertical lyrics line alignment
    VertLineAlign               m_vertLineAlign;
    int                         m_nVertLineOffset;
    int                         m_yBeginDrag;

    int getLineVertAlignPos();
public:
    void enableAdjustVertAlign(bool bEnableAdjustVertAlign) { m_bEnableAdjustVertAlign = bEnableAdjustVertAlign; }

protected:
    // Used to load and save current settings in profile
    string                      m_strSectName;

    // Event type to send when settings was changed.
    EventType                   m_etDispSettings;

    // 歌词数据
    CMLData                     *m_pMLData;
    CLyricsLines                m_lyrLines;

    bool                        m_bCanWrapLines;

    // used for auto adjust width with lyrics
    int                         m_nLeftOrg, m_nLeftAdjust;

};

cstr_t getLyrTOBSettingName(bool bHighlight, CLyricShowObj::TOBColorIndex nameIndex);

void loadLyrOverlayBlendingSettings(cstr_t szSectName, CLyricShowObj::TextOverlayBlending &tob, int nGradientPatternHeight, bool bHilight);

void createGradientFillImage24bpp(CRawImage &image, int nHeight, CColor clrGradient[3]);

void profileWriteTOBToOld(bool bOutlineLyrText,
    const CLyricShowObj::TextOverlayBlending &tobHilight,
    const CLyricShowObj::TextOverlayBlending &tobLowlight);

void profileWriteTOBCustomizedToCurrent();
