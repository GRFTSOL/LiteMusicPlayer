#include "MPlayerApp.h"
#include "MLCmd.h"
#include "MPHotkey.h"


/*
// for pocket pc

MLCmdAccKey        g_mlCmdAccKey[] = 
{
    { CMD_PLAYPAUSE, VK_RETURN, 0, VK_RETURN, 0 },
    { CMD_PLAYPAUSE, 0, 0, 0, 0 },
    { CMD_STOP, 0, 0, 0 },
    { CMD_PLAY, 0, 0, 0 },
    { CMD_PAUSE, 0, 0, 0 },
    { CMD_PREVIOUS, VK_LEFT, 0, VK_LEFT, 0 },
    { CMD_NEXT, VK_RIGHT, 0, VK_RIGHT, 0 },
    { CMD_VOL_INC, VK_UP, 0, VK_UP, 0 },
    { CMD_VOL_DEC, VK_DOWN, 0, VK_DOWN, 0 },
    { CMD_BACKWARD, 0, 0, 0, 0 },
    { CMD_FORWARD, 0, 0, 0, 0 },
    { CMD_MUTE, '#', 0, '#', 0 },
    { CMD_BACKWARD_LYRICS, 0, 0 , 0, 0 },
    { CMD_FORWARD_LYRICS, 0, 0, 0, 0 },
    { CMD_TOGGLE_SCREEEN, 0, 0, 0, 0 },
    { CMD_SHUFFLE, 0, 0, 0, 0 },
    { CMD_LOOP, 0, 0, 0, 0 },
};

int                g_mlCmdAccKeyCount = CountOf(g_mlCmdAccKey) - 1;*/


int g_hcPlayback[] = { CMD_PLAY, CMD_PLAYPAUSE, CMD_STOP, CMD_PREVIOUS, CMD_NEXT, CMD_FORWARD, CMD_BACKWARD, CMD_J_PREV_LINE, CMD_J_NEXT_LINE, 0 };
int g_hcGeneral[] = { CMD_HELP, CMD_QUIT, CMD_CLOSE, CMD_MENU, CMD_PREFERENCES, CMD_DISPLAY_OPT, 0 };
int g_hcUI[] = { CMD_FONT_SIZE_INC, CMD_FONT_SIZE_DEC, CMD_CLR_PREV_HUE, CMD_CLR_NEXT_HUE, CMD_TOGGLE_MP, CMD_MINIMIZE, CMD_TOPMOST, CMD_CLICK_THROUGH, 0 };
int g_hcLyrics[] = { CMD_OPEN_LRC, CMD_BACKWARD_LYRICS, CMD_FORWARD_LYRICS, CMD_RELOAD_LYR, 0 };
int g_hcLyrEditor[] = { CMD_LYR_EDITOR, CMD_NEW_LRC, CMD_SAVE_LRC, CMD_SAVE_LRC_AS, CMD_SAVE_LYR_IN_SONG_FILE, CMD_INSERTTAG_DOWN, CMD_INSERTTAG, CMD_DEL_TAG, CMD_AUTO_FILL_LYR_INFO, CMD_JUMP, CMD_FORWARD_CUR_LINE, CMD_BACKWARD_CUR_LINE, CMD_FORWARD_REMAIN_LINES, CMD_BACKWARD_REMAIN_LINES, CMD_REMOVE_ALL_TAG, CMD_REMOVE_BLANK_LINE, CMD_TRIM_WHITESPACE, CMD_REMOVE_UNSYNC_LINES, CMD_CAPITALIZE_LEADING_LETTER, CMD_LYRICS_TO_LOWERCASE, 0 };

MPHotKeySection        g_vHotkeySections[] = {
    { _TLM("Playback"), g_hcPlayback },
    { _TLM("General"), g_hcGeneral },
    { _TLM("UI"), g_hcUI },
    { _TLM("Lyrics"), g_hcLyrics },
    { _TLM("Lyrics Editor"), g_hcLyrEditor },
    { nullptr, nullptr }
};

#ifndef VK_MEDIA_NEXT_TRACK
#define VK_MEDIA_NEXT_TRACK 0xB0
#define VK_MEDIA_PREV_TRACK 0xB1
#define VK_MEDIA_STOP       0xB2
#define VK_MEDIA_PLAY_PAUSE 0xB3
#endif

#define MOD_ALT             0x0001
#define MOD_CONTROL         0x0002
#define MOD_SHIFT           0x0004

CMPHotkey::CmdAccKey            g_vDefAccKey[] = {
    { CMD_PLAYPAUSE, true, VK_MEDIA_PLAY_PAUSE, 0, 0 },
    { CMD_STOP, true, VK_MEDIA_STOP, 0, 0 },
    { CMD_NEXT, true, VK_MEDIA_NEXT_TRACK, 0, 0 },
    { CMD_PREVIOUS, true, VK_MEDIA_PREV_TRACK, 0, 0 },

    { CMD_BACKWARD_LYRICS, false, VK_UP, MOD_CONTROL | MOD_SHIFT, 0 },
    { CMD_FORWARD_LYRICS, false, VK_DOWN, MOD_CONTROL | MOD_SHIFT, 0 },
    { CMD_TOPMOST, true, 'T', MOD_CONTROL | MOD_SHIFT, 0 },
    { CMD_TOGGLE_MP, true, 'M', MOD_CONTROL | MOD_ALT, 0 },

    { CMD_HELP, false, VK_F1, 0, 0 },

    { CMD_OPEN_LRC, false, 'D', MOD_CONTROL, 0 },
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

    { CMD_MINIMIZE, false, 'M', MOD_CONTROL, 0 },
    { CMD_TOPMOST, false, 'T', MOD_CONTROL, 0 },

    { CMD_PREFERENCES, false, 'P', MOD_CONTROL, 0 },

};

#define SZ_HOTKEY_FILE_HEADER_V1 "HotkeysV1.0"
#define SZ_HOTKEY_FILE_HEADER   "HotkeysV1.1"
#define SZ_HOTKEY_FILENAME      "hotkeys.cfg"

inline int makeHotkeyID(int nModifier, int button) {
    return 0xC000 | (nModifier << 8) | button;
}



CMPHotkey::CMPHotkey() {
    m_bGlobalHotkeyEnabled = false;
}

CMPHotkey::~CMPHotkey() {

}

void CMPHotkey::init() {
    m_bGlobalHotkeyEnabled = g_profile.getBool("enableGlobalHotkey", false);

    if (loadSettings() != ERR_OK) {
        // load default
        m_vAccKey.clear();
        for (int i = 0; i < CountOf(g_vDefAccKey); i++) {
            m_vAccKey.push_back(g_vDefAccKey[i]);
            m_vAccKey.back().idHotKey = makeHotkeyID(g_vDefAccKey[i].fsModifiers, g_vDefAccKey[i].button);
        }
    }
}

void CMPHotkey::setEventWnd(Window *pWnd) {
    if (m_bGlobalHotkeyEnabled) {
        registerAllGlobalHotKeys();
    }
}

void CMPHotkey::quit() {
    // free hotkeys...
    unregisterAllGlobalHotKeys();
    m_vAccKey.clear();
}

void CMPHotkey::enableGlobalHotkey(bool bEnable) {
    if (m_bGlobalHotkeyEnabled == bEnable) {
        return;
    }

    g_profile.writeInt("enableGlobalHotkey", bEnable);

    m_bGlobalHotkeyEnabled = bEnable;

    if (m_bGlobalHotkeyEnabled) {
        registerAllGlobalHotKeys();
    } else {
        unregisterAllGlobalHotKeys();
    }
}

void CMPHotkey::restoreDefaults() {
    unregisterAllGlobalHotKeys();
    m_vAccKey.clear();

    for (int i = 0; i < CountOf(g_vDefAccKey); i++) {
        m_vAccKey.push_back(g_vDefAccKey[i]);
        m_vAccKey.back().idHotKey = makeHotkeyID(g_vDefAccKey[i].fsModifiers, g_vDefAccKey[i].button);
    }

    if (m_bGlobalHotkeyEnabled) {
        registerAllGlobalHotKeys();
    }

    saveSettings();
}

void CMPHotkey::onHotKey(int nId, uint32_t fuModifiers, uint32_t uVirtKey) {
    for (int i = 0; i < (int)m_vAccKey.size(); i++) {
        CmdAccKey &cmdKey = m_vAccKey[i];

        if (cmdKey.bGlobal && nId == cmdKey.idHotKey) {
            // execute cmd
            if (CMPlayerAppBase::getMPSkinFactory()->getMainWnd()) {
                CMPlayerAppBase::getMPSkinFactory()->getMainWnd()->postShortcutKeyCmd(cmdKey.cmd);
            }
            return;
        }
    }
}

bool CMPHotkey::onKeyDown(CMPSkinWnd *pWnd, uint32_t nChar, uint32_t nFlags) {
    assert(pWnd);

    bool ctrl = isModifierKeyPressed(MK_CONTROL, nFlags);
    bool shift = isModifierKeyPressed(MK_SHIFT, nFlags);
    bool alt = isModifierKeyPressed(MK_ALT, nFlags);

    int nModifiers = 0;
    int nIDKeyPressed;

    if (ctrl) {
        nModifiers |= MOD_CONTROL;
    }
    if (shift) {
        nModifiers |= MOD_SHIFT;
    }
    if (alt) {
        nModifiers |= MOD_ALT;
    }

    nIDKeyPressed = makeHotkeyID(nModifiers, nChar);

    for (int i = 0; i < (int)m_vAccKey.size(); i++) {
        CmdAccKey &cmdKey = m_vAccKey[i];

        if (!cmdKey.bGlobal && nIDKeyPressed == cmdKey.idHotKey) {
            // execute cmd
            pWnd->postShortcutKeyCmd(cmdKey.cmd);
            return true;
        }
    }

    return false;
}

void CMPHotkey::add(int cmd, bool bGlobal, int nKey, int fuModifiers) {
    CmdAccKey cmdKey;
    cmdKey.cmd = cmd;
    cmdKey.bGlobal = bGlobal;
    cmdKey.button = nKey;
    cmdKey.fsModifiers = fuModifiers;
    cmdKey.idHotKey = makeHotkeyID(cmdKey.fsModifiers, cmdKey.button);

    m_vAccKey.push_back(cmdKey);

    saveSettings();
}

bool CMPHotkey::set(int nIndex, int cmd, bool bGlobal, int nKey, int fuModifiers) {
    if (nIndex < 0 || nIndex >= (int)m_vAccKey.size()) {
        return false;
    }

    CmdAccKey &cmdKey = m_vAccKey[nIndex];
    if (cmdKey.button != 0 && cmdKey.bGlobal) {

    }
    cmdKey.cmd = cmd;
    cmdKey.bGlobal = bGlobal;
    cmdKey.button = nKey;
    cmdKey.fsModifiers = fuModifiers;
    cmdKey.idHotKey = makeHotkeyID(cmdKey.fsModifiers, cmdKey.button);

    saveSettings();

    return true;
}

bool CMPHotkey::remove(int nIndex) {
    if (nIndex < 0 || nIndex >= (int)m_vAccKey.size()) {
        return false;
    }

    // CmdAccKey &cmdKey = m_vAccKey[nIndex];
    m_vAccKey.erase(m_vAccKey.begin() + nIndex);

    saveSettings();

    return true;
}

CMPHotkey::CmdAccKey *CMPHotkey::get(int nIndex) {
    if (nIndex < 0 || nIndex >= (int)m_vAccKey.size()) {
        return nullptr;
    }

    return &(m_vAccKey[nIndex]);
}

int CMPHotkey::getByKey(int nKey, int nModifiers) {
    int nIDKey = makeHotkeyID(nModifiers, nKey);

    for (int i = 0; i < (int)m_vAccKey.size(); i++) {
        CmdAccKey &cmdKey = m_vAccKey[i];

        if (nIDKey == cmdKey.idHotKey) {
            return i;
        }
    }

    return -1;
}

int CMPHotkey::getByCmd(int cmd, int nStartFind) {
    if (nStartFind < -1 || nStartFind >= (int)m_vAccKey.size()) {
        return -1;
    }

    for (int i = nStartFind + 1; i < (int)m_vAccKey.size(); i++) {
        CmdAccKey &cmdKey = m_vAccKey[i];

        if (cmd == cmdKey.cmd) {
            return i;
        }
    }

    return -1;
}

bool CMPHotkey::isDefaultKey(int cmd) {
    std::set<int> setKeys;
    int i;

    for (i = 0; i < (int)m_vAccKey.size(); i++) {
        CmdAccKey &cmdKey = m_vAccKey[i];

        if (cmdKey.cmd == cmd) {
            setKeys.insert(makeHotkeyID(cmdKey.fsModifiers, cmdKey.button));
        }
    }

    for (i = 0; i < CountOf(g_vDefAccKey); i++) {
        if (g_vDefAccKey[i].cmd == cmd) {
            int nId = makeHotkeyID(g_vDefAccKey[i].fsModifiers, g_vDefAccKey[i].button);
            std::set<int>::iterator it = setKeys.find(nId);
            if (it != setKeys.end()) {
                setKeys.erase(it);
            } else {
                return false;
            }
        }
    }

    return setKeys.empty();
}

void CMPHotkey::restoreDefaultKey(int cmd) {
    int i;

    for (i = 0; i < (int)m_vAccKey.size(); i++) {
        CmdAccKey &cmdKey = m_vAccKey[i];

        if (cmdKey.cmd == cmd) {
            remove(i);
            i--;
        }
    }

    for (i = 0; i < CountOf(g_vDefAccKey); i++) {
        if (g_vDefAccKey[i].cmd == cmd) {
            add(cmd, g_vDefAccKey[i].bGlobal, g_vDefAccKey[i].button, g_vDefAccKey[i].fsModifiers);
        }
    }
}

bool CMPHotkey::getHotkeyText(int cmd, string &strKey) {
    for (int i = 0; i < (int)m_vAccKey.size(); i++) {
        CmdAccKey &cmdKey = m_vAccKey[i];

        if (cmdKey.cmd == cmd) {
            return true;
        }
    }

    if (cmd == CMD_FONT_SIZE_INC) {
        strKey = string(_TLT("Ctrl+")) + _TLT("Mouse Wheel Up");
    } else if (cmd == CMD_FONT_SIZE_DEC) {
        strKey = string(_TLT("Ctrl+")) + _TLT("Mouse Wheel Down");
    } else {
        return false;
    }

    return true;
}


int CMPHotkey::loadSettings() {
    // every line is a hotkey settings.
    // cmd, keycode, modifiers
    m_vAccKey.clear();

    string strFile;
    strFile = getAppDataDir();
    strFile += SZ_HOTKEY_FILENAME;

    CTextFile file;
    if (file.open(strFile.c_str(), true, ED_SYSDEF) != ERR_OK) {
        ERR_LOG1("Failed to open hotkey config file: %s", strFile.c_str());
        return ERR_OPEN_FILE;
    }

    bool bFileVer1 = false;

    // File header correct?
    string line;
    if (!file.readLine(line)) {
        return ERR_BAD_FILE_FORMAT;
    }
    if (strcmp(line.c_str(), SZ_HOTKEY_FILE_HEADER) != 0) {
        if (strcmp(line.c_str(), SZ_HOTKEY_FILE_HEADER_V1) != 0) {
            return ERR_BAD_FILE_FORMAT;
        }
        bFileVer1 = true;
    }

    while (file.readLine(line)) {
        CmdAccKey cmdKey;
        CCommaSeparatedValues csv;
        VecStrings vValues;

        csv.split(line.c_str(), vValues);
        trimStr(vValues);

        if (vValues.size() < 3) {
            continue;
        }

        size_t index = 0;

        // Cmd
        cmdKey.cmd = CMPlayerAppBase::getMPSkinFactory()->getIDByName(vValues[index++].c_str());
        if (cmdKey.cmd == UID_INVALID) {
            continue;
        }

        // Global flag
        if (!bFileVer1) {
            cmdKey.bGlobal = tobool(stringToInt(vValues[index++].c_str()));
        }

        // keycode
        if (index < vValues.size()) {
            cmdKey.button = (uint16_t)stringToInt(vValues[index++].c_str());
        }

        // modifier
        if (index < vValues.size()) {
            cmdKey.fsModifiers = (uint16_t)stringToInt(vValues[index++].c_str());
        }

        cmdKey.idHotKey = makeHotkeyID(cmdKey.fsModifiers, cmdKey.button);
        m_vAccKey.push_back(cmdKey);
    }

    return ERR_OK;
}

int CMPHotkey::saveSettings() {
    string strFile;
    CTextFile file;

    strFile = getAppDataDir();
    strFile += SZ_HOTKEY_FILENAME;

    if (file.open(strFile.c_str(), false, ED_SYSDEF) != ERR_OK) {
        ERR_LOG1("Failed to open hotkey config file: %s", strFile.c_str());
        return ERR_OPEN_FILE;
    }

    file.writeLine(SZ_HOTKEY_FILE_HEADER);

    CCommaSeparatedValues csv;

    for (int i = 0; i < (int)m_vAccKey.size(); i++) {
        CmdAccKey &cmdKey = m_vAccKey[i];
        string strId = CMPlayerAppBase::getMPSkinFactory()->getStringOfID(cmdKey.cmd);

        if (strId.empty()) {
            continue;
        }

        csv.addValue(strId.c_str());
        csv.addValue(cmdKey.bGlobal);
        csv.addValue(cmdKey.button);
        csv.addValue(cmdKey.fsModifiers);
        file.writeLine(csv.c_str(), csv.size());

        csv.clear();
    }

    file.close();

    return ERR_OK;
}

void CMPHotkey::registerAllGlobalHotKeys() {
    for (int i = 0; i < (int)m_vAccKey.size(); i++) {
        CmdAccKey &cmdKey = m_vAccKey[i];

        cmdKey.idHotKey = makeHotkeyID(cmdKey.fsModifiers, cmdKey.button);
    }
}

void CMPHotkey::unregisterAllGlobalHotKeys() {
    for (int i = 0; i < (int)m_vAccKey.size(); i++) {
        // CmdAccKey        &cmdKey = m_vAccKey[i];

    }
}
