#pragma once

#include "PreferencePageBase.h"


void showPreferenceDialog(CSkinWnd *pParent, PreferPageID preferPageId = PAGE_UI);

void registerPreferencePage(CSkinFactory *pSkinFactory);
