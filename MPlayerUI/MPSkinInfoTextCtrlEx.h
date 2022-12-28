#pragma once

#include "LyricShowObj.h"


class CMPSkinInfoTextCtrlEx : public CLyricShowObj {
    UIOBJECT_CLASS_NAME_DECLARE(CLyricShowObj)
public:
    CMPSkinInfoTextCtrlEx();
    virtual ~CMPSkinInfoTextCtrlEx();

    void onCreate() override;

    void draw(CRawGraph *canvas) override;

    void onEvent(const IEvent *pEvent) override;

    void onLyrDrawContextChanged() override;

    void onPlayTimeChangedUpdate() override;

    void onSize() override;

    void reloadThemeOfSkin(bool bSavetoProfile) { }

protected:
    void appendInfoText(const string &str);

    void wrapInfoText();

protected:
    CLyricsLines                m_vText;
    CLyricsLines                m_vUnWrapedText;

};
