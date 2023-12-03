#pragma once

#include "PreferencePageBase.h"


class CPagePfThemeRoot : public CPagePfBase {
    UIOBJECT_CLASS_NAME_DECLARE(CPagePfBase)
public:
    CPagePfThemeRoot();

    void onInitialUpdate() override;

};

void registerPfThemePages(CSkinFactory *pSkinFactory);
