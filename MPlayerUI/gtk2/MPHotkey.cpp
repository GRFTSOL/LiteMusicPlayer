// MPHotkey.cpp: implementation of the CMPHotkey class.
//
//////////////////////////////////////////////////////////////////////

#include "../MPlayerApp.h"
#include "../MLCmd.h"
#include "MPHotkey.h"


#ifndef MOD_ALT
#define MOD_ALT         0x0001
#define MOD_CONTROL     0x0002
#define MOD_SHIFT       0x0004
#define MOD_WIN         0x0008
#endif

int                g_hcPlayback[] = { CMD_PLAY, CMD_PLAYPAUSE, CMD_STOP, CMD_PREVIOUS, CMD_NEXT, CMD_FORWARD, CMD_BACKWARD, CMD_J_PREV_LINE, CMD_J_NEXT_LINE, 0 };
int                g_hcGeneral[] = { CMD_HELP, CMD_QUIT, CMD_CLOSE, CMD_MENU, CMD_PREFERENCES, CMD_DISPLAY_OPT, CMD_COPY_SONG_FILE_LYR, 0 };
int                g_hcUI[] = { CMD_FONT_SIZE_INC, CMD_FONT_SIZE_DEC, CMD_CLR_PREV_HUE, CMD_CLR_NEXT_HUE, CMD_TOGGLE_MP, CMD_MINIMIZE, CMD_TOPMOST, CMD_CLICK_THROUGH, 0 };
int                g_hcLyrics[] = { CMD_SEARCH_LYRICS, CMD_BACKWARD_LYRICS, CMD_FORWARD_LYRICS, CMD_RELOAD_LYR, 0 };
int                g_hcLyrEditor[] = { CMD_LYR_EDITOR, CMD_NEW_LRC, CMD_OPEN_LRC, CMD_SAVE_LRC, CMD_SAVE_LRC_AS, CMD_SAVE_LYR_IN_SONG_FILE, CMD_INSERTTAG_DOWN, CMD_INSERTTAG, CMD_DEL_TAG, CMD_AUTO_FILL_LYR_INFO, CMD_JUMP, CMD_FORWARD_CUR_LINE, CMD_BACKWARD_CUR_LINE, CMD_FORWARD_REMAIN_LINES, CMD_BACKWARD_REMAIN_LINES, CMD_REMOVE_ALL_TAG, CMD_REMOVE_BLANK_LINE, CMD_TRIM_WHITESPACE, CMD_REMOVE_UNSYNC_LINES, CMD_CAPITALIZE_LEADING_LETTER, CMD_LYRICS_TO_LOWERCASE, 0 };

MPHotKeySection        g_vHotkeySections[] = 
{
    { "Playback:", g_hcPlayback },
    { "General:", g_hcGeneral },
    { "UI:", g_hcUI },
    { "Lyrics:", g_hcLyrics },
    { "Lyrics Editor:", g_hcLyrEditor },
    { nullptr, nullptr }
};

CMPHotkey::CmdAccKey            g_vDefAccKey[] = 
{
    { CMD_BACKWARD_LYRICS, true, VK_UP, MOD_CONTROL | MOD_SHIFT, 0 },
    { CMD_FORWARD_LYRICS, true, VK_DOWN, MOD_CONTROL | MOD_SHIFT, 0 },
    { CMD_TOPMOST, true, 'T', MOD_CONTROL | MOD_SHIFT, 0 },
    { CMD_TOGGLE_MP, true, 'M', MOD_CONTROL | MOD_ALT, 0 },

    { CMD_HELP, false, VK_F1, 0, 0 },

    { CMD_SEARCH_LYRICS, false, 'D', MOD_CONTROL, 0 },
    { CMD_RELOAD_LYR, false, VK_F5, 0, 0 },

    { CMD_LYR_EDITOR, false, 'E', MOD_CONTROL, 0 },
    { CMD_NEW_LRC, false, 'N', MOD_CONTROL, 0 },
    { CMD_OPEN_LRC, false, 'O', MOD_CONTROL, 0 },
    { CMD_SAVE_LRC, false, 'S', MOD_CONTROL, 0 },
    { CMD_JUMP, false, 'J', MOD_CONTROL, 0 },
    { CMD_FORWARD_REMAIN_LINES, false, VK_F11, 0, 0 },
    { CMD_BACKWARD_REMAIN_LINES, false, VK_F12, 0, 0 },
    { CMD_INSERTTAG_DOWN, false, VK_F7, 0, 0 },
    { CMD_INSERTTAG, false, VK_F7, MOD_CONTROL, 0 },
    { CMD_FORWARD_CUR_LINE, false, VK_ADD, MOD_CONTROL, 0 },
    { CMD_FORWARD_CUR_LINE, false, 187, MOD_CONTROL, 0 },
    { CMD_BACKWARD_CUR_LINE, false, VK_SUBTRACT, MOD_CONTROL, 0 },
    { CMD_BACKWARD_CUR_LINE, false, 189, MOD_CONTROL, 0 },

    { CMD_FORWARD, false, VK_NEXT, MOD_CONTROL, 0 },
    { CMD_BACKWARD, false, VK_PRIOR, MOD_CONTROL, 0 },
    { CMD_PLAYPAUSE, false, 'P', MOD_CONTROL, 0 },

    { CMD_MINIMIZE, false, 'M', MOD_CONTROL, 0 },
    { CMD_TOPMOST, false, 'T', MOD_CONTROL, 0 },

    { CMD_PREFERENCES, false, 'P', MOD_CONTROL, 0 },
    { CMD_COPY_SONG_FILE_LYR, false, VK_F12, MOD_CONTROL, 0 },

};

#define SZ_HOTKEY_FILE_HEADER_V1    "HotkeysV1.0"
#define SZ_HOTKEY_FILE_HEADER        "HotkeysV1.1"
#define SZ_HOTKEY_FILENAME            "hotkeys.cfg"

inline int makeHotkeyID(int nModifier, int button)
{
    return 0xC000 | (nModifier << 8) | button;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMPHotkey::CMPHotkey()
{
}

CMPHotkey::~CMPHotkey()
{

}

void CMPHotkey::init()
{
}

void CMPHotkey::setEventWnd(Window *pWnd)
{
}

void CMPHotkey::quit()
{
}

void CMPHotkey::enableGlobalHotkey(bool bEnable)
{
}

void CMPHotkey::restoreDefaults()
{
}

void CMPHotkey::onHotKey(int nId, uint32_t fuModifiers, uint32_t uVirtKey)
{
}

bool CMPHotkey::onKeyDown(CMPSkinWnd *pWnd, uint32_t nChar, uint32_t nFlags)
{
    return false;
}

void CMPHotkey::add(int cmd, bool bGlobal, int nKey, int fuModifiers)
{
}

bool CMPHotkey::set(int nIndex, int cmd, bool bGlobal, int nKey, int fuModifiers)
{
    return true;
}

bool CMPHotkey::remove(int nIndex)
{
}

CMPHotkey::CmdAccKey *CMPHotkey::get(int nIndex)
{
    return nullptr;
}

int CMPHotkey::getByKey(int nKey, int nModifiers)
{
    return -1;
}

bool CMPHotkey::getHotkeyText(int cmd, string &strKey)
{
    return false;
}


int CMPHotkey::loadSettings()
{
    return ERR_OK;
}

int CMPHotkey::saveSettings()
{
    return ERR_OK;
}

void CMPHotkey::registerAllHotKeys()
{
}

void CMPHotkey::unregisterAllHotKeys()
{
}
