
#pragma once

#include "SkinScrollFrameCtrlBase.h"
#include "UndoMgr.h"
#include "../Utils/StringIterator.h"


class CSkinEditCtrl;

//
// Editor syntax (color) parser
//
class IEditSyntaxParser
{
public:
    virtual ~IEditSyntaxParser() { }
    virtual void init(CSkinEditCtrl *pEdit) = 0;
    virtual void onNotifyParseLine(int nLine) = 0;

};


class IEditNotification
{
public:
    enum Status
    {
        S_CAN_UNDO,
        S_CAN_REDO,
        S_SEL,
    };

    enum SpecialKey
    {
        SK_ENTER,
        SK_CTRL_ENTER,
        SK_SHIFT_ENTER,
        SK_ESCAPE,
        SK_TAB,
    };

    virtual void onStatusChanged(Status status, bool bVal) { }
    virtual void onTextChanged() { }

    virtual void onSpecialKey(SpecialKey key) { }

};

#define CARET_EXTRA_EXPAND        2

class CSkinCaret
{
public:
    CSkinCaret()
    {
        m_bOldDrawed = false;
        m_rcCaret.setEmpty();
        m_bOn = false;
        m_clrCaret.set(RGB(255, 255, 255));
        m_xOffset = 0;
        m_yOffset = 0;
    }

    virtual ~CSkinCaret() { }

    bool isOn() { return m_bOn; }

    bool isDrawed() { return m_bOldDrawed; }

    virtual void drawFlash(CRawGraph *canvas)
    {
        if (!m_bOn)
            return;

        canvas->fillRectXOR(&m_rcCaret, m_clrCaret);
        m_bOldDrawed = !m_bOldDrawed;
    }

    virtual void draw(CRawGraph *canvas)
    {
        if (!m_bOn)
            return;

        if (m_bOldDrawed)
            canvas->fillRectXOR(&m_rcCaret, m_clrCaret);
    }

    virtual void showCaret(int x, int y, int nWidth, int nHeight)
    {
        m_bOn = true;
        x += m_xOffset;
        y += m_yOffset;
        m_rcCaret.setLTRB(x, y, x + nWidth, y + nHeight + CARET_EXTRA_EXPAND);
    }

    void hideCaret()
    {
        m_bOn = false;
        m_bOldDrawed = false;
    }

    void setOffset(int xOffset, int yOffset)
    {
        if (xOffset != m_xOffset || m_yOffset != yOffset)
        {
            m_rcCaret.offsetRect(xOffset - m_xOffset, yOffset - m_yOffset);
            m_xOffset = xOffset;
            m_yOffset = yOffset;
        }
    }

    const CRect & getUpdateRect() { return m_rcCaret; }

protected:
    bool            m_bOn;
    CColor            m_clrCaret;

    CRect            m_rcCaret;

    bool            m_bOldDrawed;
    int                m_xOffset, m_yOffset;

};


class CSkinEditCtrl : public CSkinScrollFrameCtrlBase, public IUndoMgrNotify
{
    UIOBJECT_CLASS_NAME_DECLARE(CSkinScrollFrameCtrlBase)
public:
    enum ColorName
    {
        CN_BG,
        CN_TEXT,
        CN_SEL_BG,
        CN_SEL_TEXT,
        CN_EDIT_LINE_BG,
        CN_EDIT_LINE_TEXT,
        CN_CUSTOMIZED_START,

        CN_MAX = 255
    };

    enum
    {
        TIMER_DURATION_CARET        = 600
    };

public:
    class CAutoUpdate
    {
    public:
        CAutoUpdate(CSkinEditCtrl *pTarg)
        {
            m_pTarget = pTarg;
            m_bUpdate = false;
            saveOrg();
        }
        ~CAutoUpdate()
        {
            doUpdateNow();
        }
        void setUpdateFlag(bool bUpdate) { m_bUpdate = bUpdate; }

        void doUpdateNow()
        {
            if (!m_pTarget)
                return;

            if (m_pTarget->isInBatchAction())
                return;

            if (m_bUpdate)
            {
                m_bUpdate = false;
                saveOrg();
                m_pTarget->invalidate();
                return;
            }

            if (m_nTopVisibleLineOld != m_pTarget->m_nTopVisibleLine ||
                m_nScrollPosxOld != m_pTarget->m_nScrollPosx)
            {
                saveOrg();
                m_pTarget->invalidate();
                return;
            }

            if (m_nCaretRowOld != m_pTarget->m_nCaretRow)
            {
                saveOrg();
                m_pTarget->invalidate();
                return;
            }

            if (m_nBegSelRowOld == -1 && m_pTarget->m_nBegSelRow == -1)
            {
                saveOrg();
                return;
            }

            if (m_nBegSelRowOld != m_pTarget->m_nBegSelRow ||
                m_nEndSelRowOld != m_pTarget->m_nEndSelRow ||
                m_nBegSelColOld != m_pTarget->m_nBegSelCol ||
                m_nEndSelColOld != m_pTarget->m_nEndSelCol)
            {
                saveOrg();
                m_pTarget->invalidate();
                return;
            }
        }
    protected:
        void saveOrg()
        {
            m_nTopVisibleLineOld = m_pTarget->m_nTopVisibleLine;
            m_nScrollPosxOld = m_pTarget->m_nScrollPosx;
            m_nBegSelRowOld = m_pTarget->m_nBegSelRow;
            m_nEndSelRowOld = m_pTarget->m_nEndSelRow;
            m_nBegSelColOld = m_pTarget->m_nBegSelCol;
            m_nEndSelColOld = m_pTarget->m_nEndSelCol;
            m_nCaretRowOld = m_pTarget->m_nCaretRow;
        }

    protected:
        CSkinEditCtrl*m_pTarget;
        bool        m_bUpdate;
        int            m_nTopVisibleLineOld;
        int            m_nScrollPosxOld;
        int            m_nBegSelRowOld, m_nEndSelRowOld;
        int            m_nBegSelColOld, m_nEndSelColOld;
        int            m_nCaretRowOld;

    };

    friend class CAutoUpdate;

    class CAutoBatchUndo
    {
    public:
        CAutoBatchUndo(CSkinEditCtrl *pTarg) : m_update(pTarg)
        {
            m_pTarget = pTarg;

            m_pTarget->m_undoMgr.beginBatchAction();
        }

        ~CAutoBatchUndo()
        {
            endBatchUndo();
        }

        void endBatchUndo()
        {
            m_pTarget->m_undoMgr.endBatchAction();
            m_pTarget->updateScrollInfo(true);
            m_pTarget->updateScrollInfo(false);
            m_pTarget->makeCaretInSight();
            m_update.setUpdateFlag(true);
            m_update.doUpdateNow();
        }

    protected:
        CSkinEditCtrl        *m_pTarget;
        CAutoUpdate            m_update;

    };

    friend class CAutoBatchUndo;

    struct CGlyph
    {
        uint8_t        width;
        uint8_t        clrIndex;    // color index in CSkinEditCtrl color table.
        StringIterator::CharType    chGlyph;

        bool isEqual(char ch) { return chGlyph == ch; }
        bool cmp(const char *str) { return chGlyph == *str; }
        bool iCmp(const char *str);
        bool reverseCmp(const char *str) { return chGlyph == *str; }
        bool reverseiCmp(const char *str);
        int length() { return 1; }

        // WCHAR        chGlyph;
        // bool cmp(const char *str) { return chGlyph == *str; }
        // bool iCmp(const char *str);
        // bool reverseCmp(const char *str) { return chGlyph == *str; }
        // bool reverseiCmp(const char *str);
        // int length() { return 1; }
        // void textOut(CRawGraph *canvas, int x, int y) { canvas->textOut(x, y, &chGlyph, 1); }
    };

    enum Style
    {
        S_MULTILINE            = 1,            // Multiple line editor
        S_READ_ONLY            = 1 << 0x1,        // The text can't be modified if this flag is set.
        S_PASSWORD            = 1 << 0x2,
    };

    enum NewLineType
    {
        NLT_NONE,
        NLT_RN,
        NLT_N,
        NLT_R,
    };

    class COneLine : public vector<CGlyph>
    {
    public:
        COneLine()
        {
            newLineType = NLT_NONE;
        }
        int getReturnSize()
        {
            switch (newLineType)
            {
            case NLT_N:
            case NLT_R:
                return 1;
            case NLT_RN:
                return 2;
            default:
                break;
            }
            return 0;
        }

        static cstr_t getReturnStr(NewLineType nlt)
        {
            switch (nlt)
            {
            case NLT_N: return "\n";
            case NLT_R: return "\r";
            case NLT_RN: return "\r\n";
            default:
                break;
            }
            return "";
        }

        NewLineType        newLineType;
    };

    typedef vector<COneLine*>    CVLines;

public:
    CSkinEditCtrl();
    virtual ~CSkinEditCtrl();

    //
    // Editor APIs
    //

    void setEditSyntaxParser(IEditSyntaxParser *pEditSyntaxParser) { m_pEditSyntaxParser = pEditSyntaxParser; }

    void setEditNotification(IEditNotification *pEditNotification) { m_pEditNotification = pEditNotification; }

    void replaceSel(cstr_t szText);
    void removeSel();

    void setSel(int nBeg, int nEnd);
    void setSel(int nBegLine, int nBegCol, int nEndLine, int nEndCol);
    void setSelOfLine(int nLine, int nCol, int nNumOfSel);

    void setCaret(int nRow, int nCol);
    void getCaret(int &nLine, int &nCol) { nLine = m_nCaretRow; nCol = m_nCaretCol; }

    void onFontChanged();
    void setSelColor(const CColor &clrSelText, const CColor &clrSelBg)
    { setColor(CN_SEL_TEXT, clrSelText); setColor(CN_SEL_BG, clrSelBg); }

    void setStyle(uint32_t nStyles) { m_nEditorStyles = nStyles; }
    uint32_t getStyle() const { return m_nEditorStyles; }

    virtual void setText(cstr_t szText);

    int getText(string &str);

    void selectionToText(string &str);

    int getLength();

    size_t getLineCount() const { return m_vLines.size(); }

    bool getTextOfLine(int nLine, string &strLine);
    bool setTextOfLine(int nLine, cstr_t szLine);

    bool isSelected();

    bool canUndo();
    bool canRedo();

    bool isInBatchAction() const { return m_undoMgr.isInBatchAction(); }

    void onCmdCopy();
    void onCmdCut();
    void onCmdPaste();
    void onCmdRedo();
    void onCmdUndo();
    virtual void onCmdDelete();

public:
    virtual COneLine *newOneLine() { return new COneLine; }

protected:
    virtual int getFontHeight() { return m_font.getHeight(); }

    virtual CColor & getColor(int nColorName);
    virtual void setColor(int nColorName, const CColor & clr);

    // Alpha of background, used to implement translucent effect.
    virtual uint8_t getBgAlpha();
    virtual CRawGraph *getMemGraphics()
    {
        CRawGraph        *canvas = m_pContainer->getMemGraph();
        canvas->setFont(m_font.getFont());

        return canvas;
    }

    void updateRectToScreen(const CRect *rc);

    void fillGraph(CRawGraph *canvas, const CRect *rc, int nColorName);
    void fillGraph(CRawGraph *canvas, int x, int y, int nWidth, int nHeight, int nColorName)
        { CRect        rc(x, y, x + nWidth, y + nHeight); fillGraph(canvas, &rc, nColorName); }
    void fillSelectedLineBg(CRawGraph *canvas, int x, int y, COneLine *pLine, int nBegSelCol, int nEndSelCol);

    void drawLine(CRawGraph *canvas, COneLine *pLine, int x, int xMax, int y);
    void updateWidthInfoOfLine(CRawGraph *canvas, COneLine *pLine);

    bool doSetText(cstr_t szText);

    void clear();
    void clearWidth();

    int getLineDy() { return (m_nLineSpace + getFontHeight()); }

    virtual void showCaret(int x, int y, int nWidth, int nHeight);
    virtual void hideCaret();
    virtual int getCaretLineHomePos() { return 0; }
    void showCaret();
    void makeCaretInSight();
    void setCaretToPos(int x, int y);

    int getEndColPosOfLine(int nLine);

    void nextPos(int &Row, int &nCol);
    void prevPos(int &Row, int &nCol);
    void nextWord(int &Row, int &nCol);
    void prevWord(int &Row, int &nCol);

    void selectNone();
    void removeSelected(int *pnBegSelRow = nullptr, int *pnBegSelCol = nullptr);
    void removeStr(int nRow, int nCol, int nSize);
    void removeChar(int nRow, int nCol);

    void insertChar(int nRow, int nCol, WCHAR chInsert);
    void insertStr(int nRow, int nCol, cstr_t szText, int &nRowNext, int &nColNext);

    void updateScrollInfo(bool bHorz = true, bool bVert = true);

    void getStrAtPos(int nRow, int nCol, string &str);

    bool isPosValid(int nRow, int nCol);

    void sortSelectPos(int &nBegSelRow, int &nBegSelCol, int &nEndSelRow, int &nEndSelCol, bool &bSelectedEmpty);

    bool isReadOnly() { return isFlagSet(m_nEditorStyles, S_READ_ONLY); }
    bool isSingleLine() { return !isFlagSet(m_nEditorStyles, S_MULTILINE); }
    bool isPassword() { return isFlagSet(m_nEditorStyles, S_PASSWORD); }

public:
    //
    // CUIObject messages to draw or handle user input, etc.
    //

    virtual void onCreate();

    void draw(CRawGraph *canvas);

    // rc is the destination rectangle to draw the edit
    void onSize();

    virtual void onSetFocus();
    virtual void onKillFocus();

    virtual bool onLButtonUp(uint32_t nFlags, CPoint point);
    virtual bool onLButtonDown(uint32_t nFlags, CPoint point);
    virtual bool onLButtonDblClk(uint32_t nFlags, CPoint point);
    virtual bool onMouseDrag(CPoint point);
    virtual bool onMouseMove(CPoint point);
    virtual bool onRButtonUp(uint32_t nFlags, CPoint point);
    virtual void onMouseWheel(int nWheelDistance, int nMkeys, CPoint pt);
    virtual void onKeyDown(uint32_t nChar, uint32_t nFlags);
    virtual void onChar(uint32_t nChar);

    virtual void onContexMenu(int xPos, int yPos);
    virtual bool onCommand(int nId);

    virtual void onTimer(int nId);

    virtual void onAdjustHue(float hue, float saturation, float luminance);

    virtual void onVScroll(uint32_t nSBCode, int nPos, IScrollBar *pScrollBar);
    virtual void onHScroll(uint32_t nSBCode, int nPos, IScrollBar *pScrollBar);

    void onNotifyParseLine(int nLine)
    {
        if (m_pEditSyntaxParser)
            m_pEditSyntaxParser->onNotifyParseLine(nLine);
    }

    void onNotifySelChanged()
    {
        bool    bSelected = m_nBegSelRow != -1;
        if (m_bPrevSelectedStatus != bSelected)
        {
            m_bPrevSelectedStatus = bSelected;
            if (m_pEditNotification)
                m_pEditNotification->onStatusChanged(IEditNotification::S_SEL, bSelected);
        }
    }

    bool setProperty(cstr_t szProperty, cstr_t szValue);

    virtual string &getText();

#ifdef _SKIN_EDITOR_
    void enumProperties(CUIObjProperties &listProperties);
#endif // _SKIN_EDITOR_

    //
    // IUndoMgrNotify interface
    //
    virtual void onStatusChanged(Status status, bool bVal);
    virtual void onAction(Action action);

    friend class CAutoUpdate;
    friend class CEditInsertAction;
    friend class CEditDelAction;

protected:
    uint32_t                m_nEditorStyles;

    CVLines                m_vLines;

    // scroll bar info
    int                    m_nTopVisibleLine;
    int                    m_nScrollPosx;

    int                    m_nLineSpace;

    // Selected text
    int                    m_nBegSelRow, m_nEndSelRow;
    int                    m_nBegSelCol, m_nEndSelCol;
    bool                m_bInMouseSel;        // is mouse select text?

    int                    m_nOneCharDx;

    // undo
    CUndoMgr            m_undoMgr;

    IEditSyntaxParser    *m_pEditSyntaxParser;
    IEditNotification    *m_pEditNotification;
    bool                m_bPrevSelectedStatus;

    // Caret
    int                    m_nIDTimerCaret;
    CSkinCaret            m_caret;
    int                    m_nCaretX, m_nCaretY;
    int                    m_nCaretRow, m_nCaretCol;
    int                    m_nCaretMaxXLatest;

    // Cursor
    Cursor                m_cursor;

    // Font and colors
    CSkinFontProperty    m_font;
    int                    m_xMargin, m_yMargin;
    vector<CColor>        m_vClrTable;

};
