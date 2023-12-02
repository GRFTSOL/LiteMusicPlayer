#pragma once

class CLyricShowTextEditObj;

class CLyrEditSyntaxParser : public IEditSyntaxParser {
public:
    enum LYR_CLR {
        C_LOWLIGHT                  = CSkinEditCtrl::CN_CUSTOMIZED_START,
        C_TAG,
        C_HILIGHT,
    };

    CLyrEditSyntaxParser();
    virtual ~CLyrEditSyntaxParser();

    virtual void init(CSkinEditCtrl *pEdit);
    virtual void onNotifyParseLine(int nLine);

public:
    CLyricShowTextEditObj       *m_pLyrEdit;

};

class CLyricShowTextEditObj : public CSkinEditCtrl, public IEditNotification, public IEventHandler {
    UIOBJECT_CLASS_NAME_DECLARE(CSkinEditCtrl)
public:
    CLyricShowTextEditObj();
    virtual ~CLyricShowTextEditObj();

    class CLyrOneLine : public CSkinEditCtrl::COneLine {
    public:
        CLyrOneLine() {
            beginTime = 0; endTime = 0;
            bTimeLine = false;
        }
        int                         beginTime, endTime;
        bool                        bTimeLine;
    };

public:
    enum FindTextFlag {
        FTF_DOWN                    = 0x1, // search continue from selection
        FTF_MATCH_CASE              = 0x1 << 1, // case sensitive
        FTF_WHOLEWORD               = 0x1 << 2,
    };

    bool onKeyDown(uint32_t nChar, uint32_t nFlags) override;

    void autoVScrollDown(int nDownLine);

    bool findText(cstr_t szText, uint32_t dwFlags);

    CLyrOneLine *getLine(int nLine);

    // for lyrics draw
    void fastDraw(CRawGraph *canvas, CRect *prcUpdate = nullptr);
    void reDraw(CRawGraph *canvas);
    void onPlayTimeChangedUpdate();
    void drawLineGradual(CRawGraph *canvas, COneLine *pLine, int x, int xMax, int y);

    void setColorTheme(CColor &clrHilight, CColor &clrLowlight, CColor &clrTag, CColor &clrEditLineBg);

    bool isTextChanged();

public:
    virtual void onCmdDelete() override;

    virtual COneLine *newOneLine() override { return new CLyrOneLine; }

public:
    //
    // Find and replace dialog
    //
    void showFindDialog(bool bFind = true);
    bool findNext(bool bSearchDown = true, bool isMessageOut = true);

protected:
    virtual int getCaretLineHomePos() override;

public:
    virtual void onCreate() override;
    virtual void onDestroy() override;

    virtual void onEvent(const IEvent *pEvent) override;

    virtual bool onCommand(uint32_t nId) override;

    virtual void onSaveLyrics();

    virtual void draw(CRawGraph *canvas) override;

public:
    void updateLyricsProperties(bool bRedraw = false);
    void onLyricsChanged();

public:
    virtual void onEditorTextChanged(IEditNotification::Status status, bool bVal) override;
    virtual void onEditorTextChanged() override;

    virtual void onTimer(int nId) override;

protected:
    void updateTextFontColor();

    void syncTimeTag(bool bMoveToNextLine);

    void adjustSyncTimeOfSelected(bool bIncreaseTime);

    bool                        m_bDiscardLyrChangeEvent;
    int                         m_nTimerIDUpdateLyrics;

    enum FindReplaceStatus {
        FRS_NONE,
        FRS_FIND,
        FRS_REPLACE,
    };

protected:
    CLyrEditSyntaxParser        m_lyrEditSyntaxParser;

    FindReplaceStatus           m_findReplaceStatus;
    int                         CID_MATCHCASE, CID_FIND_PREV, CID_FIND_NEXT, CID_E_FIND;
    int                         CID_REPLACE, CID_REPLACE_ALL, CID_E_REPLACE;
    int                         CID_E_ARTIST, CID_E_TITLE, CID_E_ALBUM, CID_E_BY;
    int                         CID_E_OFFSET, CID_E_MEDIA_LENGTH;
    int                         CID_RERESH_ARTIST, CID_RERESH_ALBUM, CID_RERESH_TITLE, CID_RERESH_BY, CID_RERESH_MEDIA_LENGTH;
    int                         CID_TB_SEEK_BAR;

    //
    int                         m_nTopVisibleLineOld;
    int                         m_nCurPosLineOld;
    int                         m_nDrawYOfCurPosLine;
    int                         m_nXOfCurPosLineOld;

};
