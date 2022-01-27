
#pragma once

#include "LyricShowObj.h"

class CMPSkinInfoTextCtrlEx : public CLyricShowObj
{
    UIOBJECT_CLASS_NAME_DECLARE(CLyricShowObj)
public:
    CMPSkinInfoTextCtrlEx();
    virtual ~CMPSkinInfoTextCtrlEx();

    void onCreate();

    void draw(CRawGraph *canvas);

    void onEvent(const IEvent *pEvent);

    void onLyrDrawContextChanged();

    void onPlayTimeChangedUpdate();

    void onSize();

    void reloadThemeOfSkin(bool bSavetoProfile) { }

protected:
    void appendInfoText(const string &str);

    void wrapInfoText();

protected:
    CLyricsLines        m_vText;
    CLyricsLines        m_vUnWrapedText;

};

