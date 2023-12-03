#ifndef _PREFER_PAGE_LYRICS_H_
#define _PREFER_PAGE_LYRICS_H_

#pragma once

#include "PreferencePageBase.h"


class CPagePfLyricsRoot : public CPagePfBase {
    UIOBJECT_CLASS_NAME_DECLARE(CPagePfBase)
public:
    CPagePfLyricsRoot();

    void onInitialUpdate() override;

};

void registerPfLyricsPages(CSkinFactory *pSkinFactory);

#endif // _PREFER_PAGE_LYRICS_H_
