#pragma once

#ifndef MPlayerUI_SkinTreeCtrl_h
#define MPlayerUI_SkinTreeCtrl_h


class CSkinTreeCtrlEventNotify : public IUIObjNotify {
public:
    CSkinTreeCtrlEventNotify (CUIObject *pObject) : IUIObjNotify(pObject) { cmd = C_CLICK; }
    enum Command {
        C_CLICK,
        C_DBL_CLICK,
        C_SEL_CHANGED,
        C_ENTER,
        C_KEY_DELETE,
    };

    Command                     cmd;

};

class ISkinTreeNode;

typedef vector<ISkinTreeNode*> V_SkinTreeNode;
typedef vector<int> SkinTreePath_t;
typedef vector<string> SkinTreeStrPath_t;

#define SKIN_TREE_INVALID_IMG_INDEX 0xFF

class ISkinTreeNode {
public:
    enum {
        STNF_EXPANDED               = 1,
        STNF_UPDATED                = 1 << 1,
        STNF_SELECTED               = 1 << 2,
    };

    ISkinTreeNode() {
        m_nImageIndex = 0;
        m_nExpandedImageIndex = SKIN_TREE_INVALID_IMG_INDEX;
        m_nFlags = STNF_UPDATED;
        //         m_bExpanded = false;
        //         m_bUpdated = true;
        //         m_bSelected = false;
        m_pParent = nullptr;
        m_pPrev = nullptr;
        m_pNext = nullptr;
        m_pFirstChild = nullptr;
        m_pLastChild = nullptr;
    }
    virtual ~ISkinTreeNode() {
        free();
    }

    virtual void free() {
        ISkinTreeNode *p, *pNext = nullptr;
        for (p = m_pFirstChild; p != nullptr; p = pNext) {
            pNext = p->m_pNext;
            delete p;
        }
        m_pFirstChild = nullptr;
        m_pLastChild = nullptr;
    }

    void addChildBack(ISkinTreeNode *pChild);
    void addChildFront(ISkinTreeNode *pChild);
    void removeLink();

    ISkinTreeNode *firstChild() { return m_pFirstChild; }
    ISkinTreeNode *lastChild() { return m_pLastChild; }
    ISkinTreeNode *prevSibling() { return m_pPrev; }
    ISkinTreeNode *nextSibling() { return m_pNext; }

    ISkinTreeNode *getChildByIndex(int nIndex);
    ISkinTreeNode *getChildByName(cstr_t szName);
    int getDepth();
    bool isBigBrother();
    bool noChild() { return m_pFirstChild == nullptr; }
    bool hasChild() { return m_pFirstChild != nullptr; }

    void setExpanded(bool bExpanded) { if (bExpanded) m_nFlags |= STNF_EXPANDED; else m_nFlags &= ~STNF_EXPANDED; }
    bool isExpanded() { return (m_nFlags & STNF_EXPANDED) != 0; }

    void setUpdated(bool bUpdated) { if (bUpdated) m_nFlags |= STNF_UPDATED; else m_nFlags &= ~STNF_UPDATED; }
    bool isUpdated() { return (m_nFlags & STNF_UPDATED) != 0; }

    void setSelected(bool bSelcted) { if (bSelcted) m_nFlags |= STNF_SELECTED; else m_nFlags &= ~STNF_SELECTED; }
    bool isSelected() { return (m_nFlags & STNF_SELECTED) != 0; }

    void getPath(SkinTreePath_t &path);
    void getPath(SkinTreeStrPath_t &path);

    // visible == expanded node
    int getExpandedNodeCount();
    int getExpandedNodeCountTill(ISkinTreeNode *pNodeTill, bool &bFound);
    ISkinTreeNode *getExpandedNodeByPos(int &nPos);

    // ISkinTreeNode *LastVisibleNode();
    // Tree notify messages...
    virtual void onUpdate() { }

public:
    string                      m_strName;
    uint8_t                     m_nImageIndex;
    uint8_t                     m_nExpandedImageIndex;
    uint16_t                    m_nFlags;
    //     bool                    m_bExpanded;
    //     bool                    m_bSelected;
    //     bool                    m_bUpdated;
    ISkinTreeNode               *m_pParent, *m_pPrev, *m_pNext, *m_pFirstChild, *m_pLastChild;

};

class ISkinTree {
public:
    ISkinTree();
    virtual ~ISkinTree();

    virtual void getCurPath(SkinTreePath_t &path);
    virtual void getCurPath(SkinTreeStrPath_t &path);
    virtual ISkinTreeNode *getSelNode();
    virtual ISkinTreeNode *getFirstVisibleNode();

    int getAllExpandedNodeCount();
    int getFirstVisibleNodePos();

    int getExpandedNodePos(ISkinTreeNode *pNode);
    ISkinTreeNode *getExpandedNodeByPos(int nPos);

    // if selected node changed, return true.
    bool setSelNode(ISkinTreeNode *pNode);
    void setFirstVisibleNode(ISkinTreeNode *pNode);

    ISkinTreeNode *getRoot() { return &m_root; }

    ISkinTreeNode *getNode(SkinTreePath_t &path);
    ISkinTreeNode *getNode(SkinTreeStrPath_t &path);

public:
    // Tree node visitor
    static ISkinTreeNode *centerOrderNextExpandedOnly(ISkinTreeNode *pNode);

protected:
    ISkinTreeNode               m_root;             // Root Node isn't Visible
    SkinTreePath_t              m_pathSel;
    SkinTreePath_t              m_pathFirstVisible;

};

class CSkinTreeCtrl : public CUIObject, public IScrollNotify {
    UIOBJECT_CLASS_NAME_DECLARE(CUIObject)
public:
    enum TreeArea {
        TA_BLANK,
        TA_LINE,
        TA_EXPAND,
    };

    CSkinTreeCtrl();
    ~CSkinTreeCtrl();

    virtual void setDataSrc(ISkinTree *pTree) { m_pTreeData = pTree; }

    void draw(CRawGraph *canvas) override;

    TreeArea hitTest(CPoint point, ISkinTreeNode **ppNode);

    void onKeyDown(uint32_t nChar, uint32_t nFlags) override;

    bool onLButtonUp(uint32_t nFlags, CPoint point) override;
    bool onLButtonDown(uint32_t nFlags, CPoint point) override;
    bool onLButtonDblClk(uint32_t nFlags, CPoint point) override;
    void onMouseWheel(int nWheelDistance, int nMkeys, CPoint pt) override;

    void onCreate() override;
    void onSize() override;

    virtual void onVScroll(uint32_t nSBCode, int nPos, IScrollBar *pScrollBar) override;
    virtual void onHScroll(uint32_t nSBCode, int nPos, IScrollBar *pScrollBar) override;


    virtual bool setProperty(cstr_t szProperty, cstr_t szValue) override;

#ifdef _SKIN_EDITOR_
    void enumProperties(CUIObjProperties &listProperties);
#endif // _SKIN_EDITOR_

protected:
    void setBkColor(const CColor &clrBk);
    void setSelRowBkColor(const CColor &clrBk);
    void setFont(const CFontInfo &font);
    void setTextColor(const CColor &clrText);

    void notifyEvent(CSkinTreeCtrlEventNotify::Command cmd);

    void setSelNode(ISkinTreeNode *pNode);
    void setFirstVisibleNode(ISkinTreeNode *pNode);

    void updateVScrollBar();

    void makeSureSelNodeVisible();

protected:
    ISkinTree                   *m_pTreeData;

    IScrollBar                  *m_pScrollBar;
    CUIObject                   *m_pObjScrollBar;

    CSFImgList                  m_imageList;
    string                      m_strImageList;

    CColor                      m_clrBk, m_clrSelRowBk;
    CColor                      m_clrSelText;
    CSkinFontProperty           m_font;
    int                         m_nLineHeight;
    int                         m_nXMargin;
    int                         m_nLineIndent;

};

#endif // !defined(MPlayerUI_SkinTreeCtrl_h)
