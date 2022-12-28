#pragma once

#include "PreferencePageBase.h"


class CPagePfUIRoot : public CPagePfBase {
    UIOBJECT_CLASS_NAME_DECLARE(CPagePfBase)
public:
    CPagePfUIRoot();

    void onInitialUpdate() override;

};

void registerPfUIPages(CSkinFactory *pSkinFactory);
