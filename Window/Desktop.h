#pragma once

#ifndef _DESKTOP_INC_
#define _DESKTOP_INC_

class Window;

void openUrl(Window *pWnd, cstr_t szUrl);

// For mac os x, the nFlags is the nFlags in onKeyDown parameters.
bool isModifierKeyPressed(int nKey, uint32_t nFlags = 0);

CPoint getCursorPos();

bool setCursor(Cursor &Cursor);

Window *findWindow(cstr_t szClassName, cstr_t szWindowName);

bool getMonitorRestrictRect(const CRect &rcIn, CRect &rcRestrict);

bool copyTextToClipboard(cstr_t szText);
bool getClipBoardText(string &str);

#endif // !defined(_DESKTOP_INC_)
