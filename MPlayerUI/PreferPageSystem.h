#pragma once

#include "PreferencePageBase.h"


class CPagePfSystemRoot : public CPagePfBase {
    UIOBJECT_CLASS_NAME_DECLARE(CPagePfBase)
public:
    CPagePfSystemRoot();

    void onInitialUpdate() override;

    // bool onCustomCommand(int nId);

};

void registerPfSystemPages(CSkinFactory *pSkinFactory);
