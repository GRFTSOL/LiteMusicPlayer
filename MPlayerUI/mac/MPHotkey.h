#pragma once

#ifndef MPlayerUI_mac_MPHotkey_h
#define MPlayerUI_mac_MPHotkey_h


string formatHotkeyText(uint32_t key, uint32_t modifiers);

struct MPHotKeySection {
    cstr_t                      szName;
    int                         *vHotkeys;
};

typedef struct OpaqueEventHotKeyRef*    EventHotKeyRef;

extern MPHotKeySection g_vHotkeySections[];

class CMPHotkey {
public:
    struct CmdAccKey {
        int                         cmd = 0;
        bool                        bGlobal = false;
        uint16_t                    button = 0;
        uint32_t                    fsModifiers = 0;
        uint32_t                    idHotKey = 0;
        EventHotKeyRef              hotKeyRef = nullptr;
    };
    typedef vector<CmdAccKey>        VecCmAccKeys;
    typedef VecCmAccKeys::iterator   iterator;

public:
    CMPHotkey();
    virtual ~CMPHotkey();

    void init();
    void quit();

    void setEventWnd(Window *pWnd);

    void enableGlobalHotkey(bool bEnable);

    void restoreDefaults();

    void onHotKey(uint32_t nId, uint32_t fuModifiers, uint32_t uVirtKey);

    // If this key was processed, return true.
    bool onKeyDown(CMPSkinWnd *pWnd, uint32_t nChar, uint32_t nFlags);

    void add(int cmd, bool bGlobal, int nKey, int fuModifiers);
    bool set(int nIndex, int cmd, bool bGlobal, int nKey, int fuModifiers);
    bool remove(int nIndex);
    CmdAccKey *get(int nIndex);
    int getByKey(int nKey, int nModifiers);
    int getByCmd(int cmd, int nStartFind = -1);

    bool isDefaultKey(int cmd);
    void restoreDefaultKey(int cmd);

    bool getHotkeyText(int cmd, string &strKey);

    iterator begin() { return m_vAccKey.begin(); }
    iterator end() { return m_vAccKey.end(); }

protected:
    int loadSettings();
    int saveSettings();

    void registerAllGlobalHotKeys();
    void unregisterAllGlobalHotKeys();

protected:
    bool                        m_bGlobalHotkeyEnabled;
    VecCmAccKeys                m_vAccKey;

    struct _HotKeyPrivate       *m_data;

};


#endif // !defined(MPlayerUI_mac_MPHotkey_h)
