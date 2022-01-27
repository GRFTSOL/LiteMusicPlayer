// MPSkinWnd.h: interface for the CMPSkinWnd class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MPSKINWND_H__C466015C_6B75_4118_98E2_AEDA64F864DA__INCLUDED_)
#define AFX_MPSKINWND_H__C466015C_6B75_4118_98E2_AEDA64F864DA__INCLUDED_

#pragma once

class ISkinCmdHandler;


typedef map<CUIObject *, IEventHandler *>        MAP_EVENT_HANDLER;


class CMPSkinWnd : public CSkinWnd, public IEventHandler
{
public:
    CMPSkinWnd();
    virtual ~CMPSkinWnd();

    virtual void onPreCreate(bool &bTopmost, bool &bVisible);

    virtual void onCreate();
    virtual void onEvent(const IEvent *pEvent) { };

    void onCommand(uint32_t uID, uint32_t nNotifyCode);

    bool onCustomCommand(int nId);

    void onUIObjNotify(IUIObjNotify *pNotify);

    void onKeyDown(uint32_t nChar, uint32_t nFlags);

    void onSizeModeChanged(WndSizeMode sizeMode);

    void onVScroll(uint32_t nSBCode, int nPos, IScrollBar *pScrollBar);

    virtual void onSkinLoaded();

    virtual void onAddUIObj(CUIObject *pObj);

    virtual void onRemoveUIObj(CUIObject *pObj);

    virtual bool setProperty(cstr_t szProperty, cstr_t szValue);

    virtual void closeSkin();


public:
    // IUICheckStatus interface
    virtual bool getChecked(uint32_t nID, bool &bChecked);
    virtual bool getRadioChecked(vector<uint32_t> &vIDs, uint32_t &nIDChecked);

    virtual void postCustomCommandMsg(int nId);
    virtual void postShortcutKeyCmd(int nId);

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

    string                    m_strCmdHandler;
    LIST_SKINCMDHANDLER        m_listSkinCmdHandler;

    MAP_EVENT_HANDLER        m_mapEventHandlers;

};

inline uint8_t opaquePercentToAlpha(int nOpaquePercent)
{
    if (nOpaquePercent < 10)
        nOpaquePercent = 10;
    else if (nOpaquePercent > 100)
        nOpaquePercent = 100;
    return (uint8_t)(nOpaquePercent * 255 / 100);
}

#endif // !defined(AFX_MPSKINWND_H__C466015C_6B75_4118_98E2_AEDA64F864DA__INCLUDED_)
