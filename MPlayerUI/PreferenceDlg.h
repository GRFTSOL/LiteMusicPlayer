#pragma once

#include "PreferencePageBase.h"


void showPreferenceDialog(CSkinWnd *pParent, bool bFloatingLyr = false, PreferPageID preferPageId = PAGE_UI);

void registerPreferencePage(CSkinFactory *pSkinFactory);
