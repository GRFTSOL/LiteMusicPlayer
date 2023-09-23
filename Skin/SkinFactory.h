#pragma once

#ifndef Skin_SkinFactory_h
#define Skin_SkinFactory_h

class CSkinWnd;
class CMenu;
class CSkinFactory;
class CSkinApp;
class CSkinMenu;
class CUIObject;
class CSkinContainer;


#include "SkinResMgr.h"
#include "SkinWndDrag.h"
#include "../third-parties/rapidjson/rapidjson/document.h"


struct SkinWndStartupInfo {
    string                      strClassName, strCaptionText;
    string                      strSkinWnd;
    bool                        bMainWnd;
    CSkinWnd                    *pSkinWnd;
    Window                      *pWndParent;
    MapStrings                  mapExchangePool;

    SkinWndStartupInfo(cstr_t szClassName, cstr_t szCaptionText, cstr_t szSkinWnd,
        Window *pWndParent, bool bMainWnd = false);
};

// UIObject ID text to int
struct UIObjectIDDefinition {
    const char                  *szId;
    uint16_t                    nId;
    uint16_t                    nIdMenu;
    const char                  *szToolTip;
};

class UIObjectIDDefinitionLessCmp {
public:
    bool operator()(UIObjectIDDefinition *l1, UIObjectIDDefinition *l2) const {
        return strcmp(l1->szId, l2->szId) < 0;
    }
};

typedef set<UIObjectIDDefinition *, UIObjectIDDefinitionLessCmp> SetUIObjectIDDefinition;

enum UIObjectId {
    UID_INVALID                 = 0,
    CMD_MINIMIZE                = 1,
    CMD_MAXIMIZE,
    CMD_CLOSE,
    CMD_QUIT,
    CMD_MENU,

    // Skin Dialog
    CMD_OK,
    CMD_CANCEL,

    CMD_EXEC_FUNCTION,

    CMD_BASE_END,
    CMD_ID_CUSTOM_BASE          = 1000, // system alloc id begins from here

};

//////////////////////////////////////////////////////////////////////////////////////////////

//
// Define the map of new UIObject.
//
class IUIObjNewer {
public:
    virtual ~IUIObjNewer() { }
    virtual CUIObject *New() = 0;

};

template<class _CUIObj_t>
class CUIObjectNewer : public IUIObjNewer {
public:
    virtual ~CUIObjectNewer() { }
    virtual CUIObject *New() { return new _CUIObj_t(); }

};

class CUIObjectNewerLessCmp {
public:
    bool operator()(cstr_t s1, cstr_t s2) const
        { return strcasecmp(s1, s2) < 0; }

};

typedef map<cstr_t, IUIObjNewer *, CUIObjectNewerLessCmp> MapUIObjNewer;

#define AddUIObjNewer(_uiobj_class) m_mapUIObjNewer[_uiobj_class::className()] = new CUIObjectNewer< _uiobj_class >()
#define AddUIObjNewer2(_skinFactory, _uiobj_class)  _skinFactory->registerUIObjNewer(_uiobj_class::className(), new CUIObjectNewer< _uiobj_class >())

/////////////////////////////////////////////////////////////////////////////////////////////////////

#define _SZ_SKINWND_CLASS_NAME  "CMPSkin"

//
// 打开，保存XML格式的skin文件
class CSkinFileXML {
public:
    CSkinFileXML();
    ~CSkinFileXML();

public:
    bool isOpened() { return m_xml.m_pRoot != nullptr; }

    int load(cstr_t szFile);

    int save(cstr_t szFile);

    void close();

    SXNode *getSkinWndNode(cstr_t szSkinWndName);
    int newSkinWndNode(cstr_t szSkinWndName);
    int setSkinWndNode(cstr_t szSkinWndName, SXNode *pWndNode);
    int removeSkinWnd(cstr_t szSkinWndName);
    int changeSkinWndName(cstr_t szNameOld, cstr_t szNameNew);
    bool canUseSkinWndName(cstr_t szName);

    // get the main window of the SKIN. szSkinWndNameDef specifies the default main window.
    int getMainSkinWnd(string &strSkinWndName, cstr_t szSkinWndNameDef);

    void enumSkinWnd(vector<string> &vSkinWnd);

    SXNode *getRootNode() { return m_xml.m_pRoot; }

    SXNode *getMenusNode();
    int setMenusNode(SXNode *pNode);

    SXNode *getDynamicCmdsNode();
    int setDynamicCmdsNode(SXNode *pNode);

    cstr_t getFileName() const { return m_strFile.c_str(); }

    cstr_t getSkinProperty(cstr_t szProperty);

protected:
    int getSkinWndNode(cstr_t szSkinWndName, SXNode::iterator *itWnd);

    int setNode(cstr_t szName, SXNode *pNode);

protected:
    CSimpleXML                  m_xml;
    string                      m_strFile;

};

class CSkinStyles {
public:
    CSkinStyles();
    ~CSkinStyles();

    int fromXML(SXNode *pNodeSkin);
    void free();

    SXNode *getClassNode(cstr_t szClassName) const;

protected:
    SXNode::LIST_CHILDREN       m_listStyleNodes;

};

class CDynamicCmds {
public:
    struct DynamicCmd {
        int                         nID;
        string                      strFunction;
        string                      strParam;
    };
    typedef list<DynamicCmd>        LIST_DYNCMD;

    CDynamicCmds() { m_pSkinFactory = nullptr; }
    virtual ~CDynamicCmds() { free(); }

    void init(CSkinFactory *pSkinFactory) { m_pSkinFactory = pSkinFactory; }

    int fromXML(SXNode *pNodeDynCmds);
#ifdef _SKIN_EDITOR_
    SXNode *toXML();
#endif // _SKIN_EDITOR_

    void free();

    bool getCmd(int nCmdID, DynamicCmd &cmd);

    void enumAllCmds(VecStrings &vstrCmds);

    LIST_DYNCMD &getDataList() { return m_listDynCmd; }

protected:
    CSkinFactory                *m_pSkinFactory;
    LIST_DYNCMD                 m_listDynCmd;

};

class CSkinFactory : public ISkinWndDragHost {
public:
    CSkinFactory(CSkinApp *pApp, UIObjectIDDefinition uidDefinition[]);
    virtual ~CSkinFactory();

    virtual int init();
    virtual void quit();

    //
    // UID = UIObject ID
    //
    int getIDByName(cstr_t szId) { return getIDByNameEx(szId, nullptr); }
    int getIDByName(const string &id) { return getIDByNameEx(id.c_str(), nullptr); }
    int getIDByNameEx(cstr_t szId, string *pstrToolTip);
    virtual int getIDByNameEx(cstr_t szId, string &strToolTip)
        { return getIDByNameEx(szId, &strToolTip); }
    virtual void getTooltip(int nUID, string &strToolTip);

    int getMenuIDByUID(int nUID, bool bAllocIfInexistence = false);
    bool setMenuIDByUID(int nUID, int nMenuId);
    int getUIDByMenuID(int nMenuID);

#ifdef DEBUG
    void dumpUID();
#endif

    UIObjectIDDefinition *getUidDef(int nUID);

    int allocUID();
    string getStringOfID(int nID);

    bool isOpened() { return m_skinFile.isOpened(); }

    virtual int openSkin(cstr_t szSkinName, cstr_t szSkinDir = "", cstr_t szExtraDir = "");

    virtual int changeSkin(cstr_t szSkinName, cstr_t szSkinDir = "", cstr_t szExtraDir = "", bool bLoadSkinColorTheme = false, bool bCreateSkinWnd = true);

    virtual int saveSkinFile(cstr_t szFile);
    virtual void close(bool bQuit = false);

    CSkinWnd *findSkinWnd(cstr_t szSkinWndName);
    bool isValidSkinWnd(CSkinWnd *pWnd);

    virtual int activeOrCreateSkinWnd(SkinWndStartupInfo &skinWndStartupInfo);
    virtual int openOrCloseSkinWnd(SkinWndStartupInfo &skinWndStartupInfo);
    virtual int createSkinWnd(SkinWndStartupInfo &skinWndStartupInfo);

    virtual void onSkinWndCreate(CSkinWnd *pSkinWnd);
    virtual void onSkinWndDestory(CSkinWnd *pSkinWnd);

    virtual void onMainSkinWndDestory(CSkinWnd *pSkinWnd);

    virtual CSkinWnd *newSkinWnd(cstr_t szSkinWndName, bool bMainWnd);

    // methods for skin window
    SXNode *getSkinWndNode(cstr_t szSkinWndName);

    int loadAppResSkinWndXml(cstr_t szSkinWndName, CSimpleXML &xml);
    bool isAppResSkinWnd(cstr_t szSkinWndName);

    void registerUIObjNewer(cstr_t szClassName, IUIObjNewer *pObjNewer);

    // create new uiobject by class name
    virtual CUIObject *createUIObject(CSkinWnd *pSkin, cstr_t szClassName, CSkinContainer *pContainer);

    //
    virtual CUIObject *createDynamicCtrl(CSkinContainer *pContainer, cstr_t szClassName, int nIDAssign = UID_INVALID, cstr_t szLeft = nullptr, cstr_t szTop = nullptr, cstr_t szWidth = nullptr, cstr_t szHeight = nullptr);

    //
    // Skin root dir, and extra dir. resource files search order: SkinDir > SkinRootDir > Extra Dir
    //
    void setSkinDir(cstr_t szSkinDir);
    void setResourceExtraDir(cstr_t szExtraDir);
    void setSkinsRootDir(cstr_t szSkinsRootDir);
    cstr_t getSkinRootDir();
    void setSkinFileName(cstr_t szSkinFileName);

    bool findFirstSkin(string &strSkin);

    bool enumAllSkins(vector<string> &vSkins);

    // resource manage APIs
    CSkinResMgr *getResourceMgr() { return &m_resourceMgr; }

    CSkinWnd *getMainWnd();

    CSkinFileXML *getSkinFile() { return &m_skinFile; }

    cstr_t getSkinName() const { return m_strSkinName.c_str(); }
    cstr_t getSkinFileName() const { return m_strSkinFileName.c_str(); }

    SXNode *getExtendsStyle(cstr_t szExtendsSkinWnd) const;

    // resource
    virtual bool loadMenu(CSkinWnd *pWnd, CMenu **ppMenu, cstr_t szMenu);
    virtual void showPopupMenu(CSkinWnd *pWnd, cstr_t menuName);

    virtual int onDynamicCmd(int nCmdID, CSkinWnd *pSkinWnd);

    // hue: 0~360,        saturation: 0~1.0,        luminance: 0~1.0
    virtual void adjustHue(float hue, float saturation = 0.5, float luminance = 0.5);
    void getAdjustedHueResult(CColor &clr);
    void getAdjustedHueResult(COLORREF &clr);

    virtual void onLanguageChanged();

    virtual void onSkinWndActivate(CSkinWnd *pWnd);

    void enterInDrawUpdate();
    void leaveInDrawUpdate();

public:
    // ISkinWndDragHost
    virtual void getWndDragAutoCloseTo(vector<Window *> &vWnd);
    virtual void getWndDragTrackMove(vector<Window *> &vWnd);

protected:
    virtual int openSkinFile(cstr_t szSkinFile);

    virtual CSkinMenu *newSkinMenu(CSkinWnd *pWnd, const rapidjson::Value &items) { return nullptr; }

    CSkinMenu *loadPresetMenu(CSkinWnd *pWnd, cstr_t szMenu);

    void expandIncludeNode(SXNode *pNodeRoot);

    virtual int exeDynamicCmd(const CDynamicCmds::DynamicCmd &cmd, CSkinWnd *pSkinWnd);

    virtual CUIObject *newUIObject(CSkinWnd *pSkin, CSkinContainer *pContainer, cstr_t szClassName);

    void applyDefaultThemeOfSkin();

protected:
    string                      m_strSkinRootDir;
    string                      m_strSkinFileName;
    string                      m_strSkinName;

    CSkinFileXML                m_skinFile;

    // resource manager
    CSkinResMgr                 m_resourceMgr;

    // Skin styles
    CSkinStyles                 m_skinStyle;

    // Menus
    rapidjson::Document         m_menus;

    //
    // Dynamic commands define
    CDynamicCmds                m_dynamicCmds;

    SetUIObjectIDDefinition     m_setUIDDefinition;
    AllocatorPool               m_allocator;
    int                         m_nCustomCmdIDNext;
    int                         m_nMenuIDNext;

    MapUIObjNewer               m_mapUIObjNewer;

    typedef    list<CSkinWnd*>        ListSkinWnds;

    ListSkinWnds                m_listSkinWnds;

    CSkinApp                    *m_pApp;

};

#endif // !defined(Skin_SkinFactory_h)
