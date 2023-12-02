#pragma once

#include "PreferencePageBase.h"


class CPagePfSystemRoot : public CPagePfBase {
    UIOBJECT_CLASS_NAME_DECLARE(CPagePfBase)
public:
    CPagePfSystemRoot();

    void onInitialUpdate() override;

};

void registerPfSystemPages(CSkinFactory *pSkinFactory);
