#pragma once

#ifndef MPlayerUI_SkinDroplistCtrl_h
#define MPlayerUI_SkinDroplistCtrl_h


class IDropListCtrlNotify {
public:
    virtual void onSelChanged() = 0;
};

template<class _DataType>
class CTree {
public:
    struct Node {
        vector<Node *>              vChildren;
        _DataType                   dataNode;
    };

    typedef vector<Node *>    V_CHILDREN;

public:
    CTree() {
        m_pRootNode = new Node;
        m_pCurNode = m_pRootNode;
        m_nCurNodeIndex = 0;
    }
    ~CTree() {
        freeNode(m_pRootNode);
    }

    void clear() {
        m_vPath.clear();
        m_pCurNode = m_pRootNode;
        m_nCurNodeIndex = 0;
        eraseAllChildren();
    }

    void chToRoot() {
        m_nCurNodeIndex = 0;
        m_pCurNode = m_pRootNode;
        m_vPath.clear();
    }

    bool chToParent() {
        assert(!m_vPath.empty());
        if (m_vPath.empty()) {
            return false;
        }

        m_nCurNodeIndex = m_vPath.back();
        m_vPath.pop_back();

        if (!chToPath(m_vPath)) {
            return false;
        }

        return true;
    }

    bool chToChild(int nIndex) {
        if (nIndex < 0 || nIndex >= (int)m_pCurNode->vChildren.size()) {
            return false;
        }
        m_pCurNode = m_pCurNode->vChildren[nIndex];
        m_nCurNodeIndex = nIndex;
        m_vPath.push_back(nIndex);

        return true;
    }

    _DataType &getCurNodeData() {
        return m_pCurNode->dataNode;
    }

    _DataType &getParentNodeData() {
        Node *pNode = m_pRootNode;
        for (int i = 0; i < (int)m_vPath.size() - 1; i++) {
            if ((size_t)m_vPath[i] < pNode->vChildren.size()) {
                pNode = pNode->vChildren[m_vPath[i]];
            }
        }

        return pNode->dataNode;
    }

    int getChildrenCount() {
        return (int)m_pCurNode->vChildren.size();
    }

    _DataType &getChild(int nIndex) {
        assert(nIndex >= 0 && nIndex < (int)m_pCurNode->vChildren.size());

        if (nIndex >= 0 && nIndex < (int)m_pCurNode->vChildren.size()) {
            return m_pCurNode->vChildren[nIndex]->dataNode;
        } else {
            return m_pRootNode->dataNode;
        }
    }

    void insertChild(int nIndex, _DataType data) {
        Node *pNode = new Node;
        pNode->dataNode = data;
        m_pCurNode->vChildren.insert(m_pCurNode->vChildren.begin() + nIndex, pNode);
    }

    void addChild(_DataType data) {
        Node *pNode = new Node;
        pNode->dataNode = data;
        m_pCurNode->vChildren.push_back(pNode);
    }

    void eraseChild(int nIndex) {
        Node *pNode;

        if (nIndex >= 0 && nIndex < (int)m_pCurNode->vChildren.size()) {
            pNode = m_pCurNode->vChildren[nIndex];
            m_pCurNode->vChildren.erase(m_pCurNode->vChildren.begin() + nIndex);
            freeNode(pNode);
        }
    }

    void eraseAllChildren() {
        for (int i = 0; i < (int)m_pCurNode->vChildren.size(); i++) {
            freeNode(m_pCurNode->vChildren[i]);
        }
        m_pCurNode->vChildren.clear();
    }

    void getPath(vector<_DataType> &vPath) {
        Node *pNode = m_pRootNode;
        vPath.clear();
        for (int i = 0; i < (int)m_vPath.size(); i++) {
            if (m_vPath[i] < (int)pNode->vChildren.size()) {
                pNode = pNode->vChildren[m_vPath[i]];
                vPath.push_back(pNode->dataNode);
            }
        }
    }

protected:
    bool chToPath(vector<int> &vPath) {
        if (vPath.empty()) {
            m_pCurNode = nullptr;
            m_nCurNodeIndex = 0;
            return true;
        }

        if (vPath[0] >= (int)m_pRootNode->vChildren.size()) {
            return false;
        }
        m_pCurNode = m_pRootNode->vChildren[vPath[0]];
        m_nCurNodeIndex = vPath[0];

        for (int i = 1; i < (int)vPath.size(); i++) {
            if (vPath[i] >= (int)m_pCurNode->vChildren.size()) {
                return false;
            }
            m_pCurNode = m_pCurNode->vChildren[vPath[i]];
            m_nCurNodeIndex = vPath[i];
        }

        return true;
    }

    void freeNode(Node *pNode) {
        for (int i = 0; i < (int)pNode->vChildren.size(); i++) {
            freeNode(pNode->vChildren[i]);
        }

        delete pNode;
    }

protected:
    Node                        *m_pRootNode;
    vector<int>                 m_vPath;
    int                         m_nCurNodeIndex;
    Node                        *m_pCurNode;

};
/*
class CDropListCtrl : public Window
{
public:
    struct Item
    {
        int            nImageIndex;
        string        strText;
    };
    CDropListCtrl();
    ~CDropListCtrl();

    void setStyle(cstr_t szFontName, int nFontHeight, CColor &clrText, CColor &clrBg, CColor &clrSelBg);

    bool dropList(int x, int y, Window *pWndParent, int nID, CSFImgList *pImageList, IDropListCtrlNotify *pNotify);

    void clear();
    void append(int nImageIndex, cstr_t szText);

    void getSelPath(vector<string> &vPath);

    void onKillFocus();

    bool onKeyDown(uint32_t nChar, uint32_t nFlags);
    bool onKeyUp(uint32_t nChar, uint32_t nFlags);
    void onLButtonUp(uint32_t nFlags, CPoint point);
    void onLButtonDown(uint32_t nFlags, CPoint point);

    void onPaint(CRawGraph *canvas);

    void onDestroy();

#ifdef _WIN32
    LRESULT wndProc(uint32_t message, WPARAM wParam, LPARAM lParam);
#endif

protected:
    void paint(CGraphics *canvas, bool bResize = false);

    void notifySelChanged();

protected:
    vector<Item>        m_vItems;
    int                    m_nID;
    Window            *m_pWndParent;
    CSFImgList            *m_imageList;

    CFontInfo            m_font;
    CColor                m_clrText, m_clrBg, m_clrSelBg;

    int                    m_nLineHeight;
    int                    m_nRightMargin;
    int                    m_nSel;

    int                    m_x, m_y;

    IDropListCtrlNotify    *m_pNotify;

};

class CSkinDroplistCtrlEventNotify : public IUIObjNotify
{
public:
    CSkinDroplistCtrlEventNotify(CUIObject *pObject) : IUIObjNotify(pObject) { cmd = C_SEL_CHANGED; }
    enum Command
    {
        C_SEL_CHANGED,
    };

    Command                cmd;

};


class CSkinDroplistCtrl : public CSkinTextButton, public IDropListCtrlNotify
{
public:
    CSkinDroplistCtrl();
    virtual ~CSkinDroplistCtrl();

    cstr_t getClassName();

    void draw(CRawGraph *canvas);

    bool onLButtonUp(uint32_t nFlags, CPoint point);

    virtual void onSelChanged();

    bool setProperty(cstr_t szProperty, cstr_t szValue) override;
#ifdef _SKIN_EDITOR_
    void enumProperties(CUIObjProperties &listProperties);
#endif // _SKIN_EDITOR_

    void clear()
    {
        m_droplist.clear();
    }

    void append(int nImageIndex, cstr_t szText)
    {
        m_nImageIndex = nImageIndex;
        m_strName = szText;
        m_droplist.append(nImageIndex, szText);
    }

    void getSelPath(vector<string> &vPath)
    {
        m_droplist.getSelPath(vPath);
    }

public:
    static cstr_t className() { return ms_szClassName; }
    bool isKindOf(cstr_t szClassName);

protected:
    static cstr_t        ms_szClassName;

    CSFImgList            m_imageList;
    CDropListCtrl        m_droplist;

    int                    m_nImageIndex;

    string                m_strImgListFile;
    CColor                m_clrSelBg;

};*/

#endif // !defined(MPlayerUI_SkinDroplistCtrl_h)
