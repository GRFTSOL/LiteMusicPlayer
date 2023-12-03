#include "../MPlayerApp.h"
#include "../MLCmd.h"
#include "MPHotkey.h"


#ifndef MOD_ALT
#define MOD_ALT             0x0001
#define MOD_CONTROL         0x0002
#define MOD_SHIFT           0x0004
#define MOD_WIN             0x0008
#endif

int g_hcPlayback[] = { ID_PLAY, ID_PLAYPAUSE, ID_STOP, ID_PREVIOUS, ID_NEXT, ID_FORWARD, ID_BACKWARD, ID_J_PREV_LINE, ID_J_NEXT_LINE, 0 };
int g_hcGeneral[] = { ID_HELP, ID_QUIT, ID_CLOSE, ID_MENU, ID_PREFERENCES, ID_DISPLAY_OPT, ID_COPY_SONG_FILE_LYR, 0 };
int g_hcUI[] = { ID_FONT_SIZE_INC, ID_FONT_SIZE_DEC, ID_CLR_PREV_HUE, ID_CLR_NEXT_HUE, ID_TOGGLE_MP, ID_MINIMIZE, ID_TOPMOST, ID_CLICK_THROUGH, 0 };
int g_hcLyrics[] = { ID_SEARCH_LYRICS, ID_BACKWARD_LYRICS, ID_FORWARD_LYRICS, ID_RELOAD_LYR, 0 };
int g_hcLyrEditor[] = { ID_LYR_EDITOR, ID_NEW_LRC, ID_OPEN_LRC, ID_SAVE_LRC, ID_SAVE_LRC_AS, ID_SAVE_LYR_IN_SONG_FILE, ID_INSERTTAG_DOWN, ID_INSERTTAG, ID_DEL_TAG, ID_AUTO_FILL_LYR_INFO, ID_JUMP_TO_CUR_LINE, ID_FORWARD_CUR_LINE, ID_BACKWARD_CUR_LINE, ID_FORWARD_REMAIN_LINES, ID_BACKWARD_REMAIN_LINES, ID_REMOVE_ALL_TAG, ID_REMOVE_BLANK_LINE, ID_TRIM_WHITESPACE, ID_REMOVE_UNSYNC_LINES, ID_CAPITALIZE_LEADING_LETTER, ID_LYRICS_TO_LOWERCASE, 0 };

MPHotKeySection        g_vHotkeySections[] = {
    { "Playback:", g_hcPlayback },
    { "General:", g_hcGeneral },
    { "UI:", g_hcUI },
    { "Lyrics:", g_hcLyrics },
    { "Lyrics Editor:", g_hcLyrEditor },
    { nullptr, nullptr }
};

CMPHotkey::CmdAccKey            g_vDefAccKey[] = {
    { ID_BACKWARD_LYRICS, true, VK_UP, MOD_CONTROL | MOD_SHIFT, 0 },
    { ID_FORWARD_LYRICS, true, VK_DOWN, MOD_CONTROL | MOD_SHIFT, 0 },
    { ID_TOPMOST, true, 'T', MOD_CONTROL | MOD_SHIFT, 0 },
    { ID_TOGGLE_MP, true, 'M', MOD_CONTROL | MOD_ALT, 0 },

    { ID_HELP, false, VK_F1, 0, 0 },

    { ID_SEARCH_LYRICS, false, 'D', MOD_CONTROL, 0 },
    { ID_RELOAD_LYR, false, VK_F5, 0, 0 },

    { ID_LYR_EDITOR, false, 'E', MOD_CONTROL, 0 },
    { ID_NEW_LRC, false, 'N', MOD_CONTROL, 0 },
    { ID_OPEN_LRC, false, 'O', MOD_CONTROL, 0 },
    { ID_SAVE_LRC, false, 'S', MOD_CONTROL, 0 },
    { ID_JUMP_TO_CUR_LINE, false, 'J', MOD_CONTROL, 0 },
    { ID_FORWARD_REMAIN_LINES, false, VK_F11, 0, 0 },
    { ID_BACKWARD_REMAIN_LINES, false, VK_F12, 0, 0 },
    { ID_INSERTTAG_DOWN, false, VK_F7, 0, 0 },
    { ID_INSERTTAG, false, VK_F7, MOD_CONTROL, 0 },
    { ID_FORWARD_CUR_LINE, false, VK_ADD, MOD_CONTROL, 0 },
    { ID_FORWARD_CUR_LINE, false, 187, MOD_CONTROL, 0 },
    { ID_BACKWARD_CUR_LINE, false, VK_SUBTRACT, MOD_CONTROL, 0 },
    { ID_BACKWARD_CUR_LINE, false, 189, MOD_CONTROL, 0 },

    { ID_FORWARD, false, VK_NEXT, MOD_CONTROL, 0 },
    { ID_BACKWARD, false, VK_PRIOR, MOD_CONTROL, 0 },
    { ID_PLAYPAUSE, false, 'P', MOD_CONTROL, 0 },

    { ID_MINIMIZE, false, 'M', MOD_CONTROL, 0 },
    { ID_TOPMOST, false, 'T', MOD_CONTROL, 0 },

    { ID_PREFERENCES, false, 'P', MOD_CONTROL, 0 },
    { ID_COPY_SONG_FILE_LYR, false, VK_F12, MOD_CONTROL, 0 },

};

#define SZ_HOTKEY_FILE_HEADER_V1 "HotkeysV1.0"
#define SZ_HOTKEY_FILE_HEADER   "HotkeysV1.1"
#define SZ_HOTKEY_FILENAME      "hotkeys.cfg"

inline int makeHotkeyID(int nModifier, int button) {
    return 0xC000 | (nModifier << 8) | button;
}



CMPHotkey::CMPHotkey() {
}

CMPHotkey::~CMPHotkey() {

}

void CMPHotkey::init() {
}

void CMPHotkey::setEventWnd(Window *pWnd) {
}

void CMPHotkey::quit() {
}

void CMPHotkey::enableGlobalHotkey(bool bEnable) {
}

void CMPHotkey::restoreDefaults() {
}

void CMPHotkey::onHotKey(int nId, uint32_t fuModifiers, uint32_t uVirtKey) {
}

bool CMPHotkey::onKeyDown(CMPSkinWnd *pWnd, uint32_t nChar, uint32_t nFlags) {
    return false;
}

void CMPHotkey::add(int cmd, bool bGlobal, int nKey, int fuModifiers) {
}

bool CMPHotkey::set(int nIndex, int cmd, bool bGlobal, int nKey, int fuModifiers) {
    return true;
}

bool CMPHotkey::remove(int nIndex) {
}

CMPHotkey::CmdAccKey *CMPHotkey::get(int nIndex) {
    return nullptr;
}

int CMPHotkey::getByKey(int nKey, int nModifiers) {
    return -1;
}

bool CMPHotkey::getHotkeyText(int cmd, string &strKey) {
    return false;
}


int CMPHotkey::loadSettings() {
    return ERR_OK;
}

int CMPHotkey::saveSettings() {
    return ERR_OK;
}

void CMPHotkey::registerAllHotKeys() {
}

void CMPHotkey::unregisterAllHotKeys() {
}
