#ifndef MPlayerUI_MPSkinWnd_h
#define MPlayerUI_MPSkinWnd_h

#pragma once

class ISkinCmdHandler;


typedef map<CUIObject *, IEventHandler *> MAP_EVENT_HANDLER;


class CMPSkinWnd : public CSkinWnd, public IEventHandler {
public:
    CMPSkinWnd();
    virtual ~CMPSkinWnd();

    virtual void onPreCreate(bool &bTopmost, bool &bVisible) override;

    virtual void onCreate() override;
    virtual void onEvent(const IEvent *pEvent) override { };

    void onCommand(uint32_t uID, uint32_t nNotifyCode) override;

    bool onCustomCommand(int nId) override;

    void onUIObjNotify(IUIObjNotify *pNotify) override;

    bool onKeyDown(uint32_t nChar, uint32_t nFlags) override;

    void onSizeModeChanged(WndSizeMode sizeMode) override;

    void onVScroll(uint32_t nSBCode, int nPos, IScrollBar *pScrollBar) override;

    virtual void onSkinLoaded() override;

    virtual void onAddUIObj(CUIObject *pObj) override;

    virtual void onRemoveUIObj(CUIObject *pObj) override;

    virtual bool setProperty(cstr_t szProperty, cstr_t szValue) override;

    virtual void closeSkin() override;


public:
    // IUICheckStatus interface
    virtual bool getChecked(uint32_t nID, bool &bChecked) override;
    virtual bool getRadioChecked(vector<uint32_t> &vIDs, uint32_t &nIDChecked) override;

    virtual void postCustomCommandMsg(int nId) override;
    virtual void postShortcutKeyCmd(int nId) override;

#ifdef _WIN32_DESKTOP
protected:
    virtual void onDropFiles(HDROP hDrop);
#endif

    void addCmdHandler(cstr_t szName);

    void setTopmost(bool bTopmost);
    Window *getParentOrSelf();

    //
    // Special settings for different kind of winodw (Floating lyrics and Normal window).
    //
    virtual bool settingGetTopmost();
    virtual int settingGetOpaquePercent();
    virtual bool settingGetClickThrough();

    virtual void settingReverseTopmost();
    virtual void settingSetOpaquePercent(int nPercent);
    virtual void settingReverseClickThrough();

protected:
    typedef list<ISkinCmdHandler *>    LIST_SKINCMDHANDLER;

    string                      m_strCmdHandler;
    LIST_SKINCMDHANDLER         m_listSkinCmdHandler;

    MAP_EVENT_HANDLER           m_mapEventHandlers;

};

inline uint8_t opaquePercentToAlpha(int nOpaquePercent) {
    if (nOpaquePercent < 10) {
        nOpaquePercent = 10;
    } else if (nOpaquePercent > 100) {
        nOpaquePercent = 100;
    }
    return (uint8_t)(nOpaquePercent * 255 / 100);
}

#endif // !defined(MPlayerUI_MPSkinWnd_h)
