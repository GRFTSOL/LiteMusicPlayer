// MPEventHost.h: interface for the CMPEventHost class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MPEVENTHOST_H__AE092490_ADF2_4326_9066_B35EE136A3DD__INCLUDED_)
#define AFX_MPEVENTHOST_H__AE092490_ADF2_4326_9066_B35EE136A3DD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "../MPlayerEngine/IMPlayer.h"
#include "../Skin/Skin.h"
#include "MPlayerAppBase.h"


class CMPVisAdapter;

// section name
#define SZ_SECT_LYR_DISPLAY            "LyrDispaly"
#define SZ_SECT_FLOATING_LYR        "FloatingLyr"
#define SZ_SECT_PLAYER                "Player"
#define SZ_SECT_LYR_DL                "Download"

#ifdef _MPLAYER
#define SZ_SECT_UI            "ZikiPlayer"
#else
#define SZ_SECT_UI            "MiniLyrics"
#endif

enum MLEventType
{
    ET_INVALID = 0,
    ET_NULL = 0,

    ET_PLAYER_STATUS_CHANGED,
    ET_PLAYER_SEEK,
    ET_PLAYER_CUR_MEDIA_CHANGED,
    ET_PLAYER_CUR_MEDIA_INFO_CHANGED,
    ET_PLAYER_CUR_PLAYLIST_CHANGED,
    ET_PLAYER_SETTING_CHANGED,
    ET_PLAYER_EQ_SETTING_CHANGED,
    ET_PLAYER_PLAY_HALT_ERROR,
    ET_PLAYER_POS_UPDATE,

    ET_LYRICS_RESEARCH,
    ET_LYRICS_DISPLAY_SETTINGS,
    ET_LYRICS_FLOATING_SETTINGS,
    ET_LYRICS_DRAW_UPDATE,
    ET_LYRICS_CHANGED,
    ET_LYRICS_ON_SAVE_EDIT,
    ET_LYRICS_EDITOR_RELOAD_TAG,

    ET_LYRICS_SEARCH_END,
    ET_DOWNLOAD_END,

    ET_UI_SETTINGS_CHANGED,
    ET_UI_INFO_TEXT,
    ET_UI_LONG_ERROR_TEXT,

    ET_VIS_DRAW_UPDATE,

    COUNT_OF_EVENT_TYPE,
};

struct CEventPlayerStatusChanged : public IEvent
{
    PLAYER_STATE        status;
};

struct CEventPlayerSettingChanged : public IEvent
{
    IMPEvent::MP_SETTING_TYPE        settingType;
    int                 value;
};

struct CEventPlaylistChanged : public IEvent
{
    IMPEvent::MP_PLAYLIST_CHANGE_ACTION        action;
    int                 nIndex, nIndexOld;
};

struct CEventPlayerEQChanged : public IEvent
{
    EQualizer            eqlalizer;
};

class CEventLyricsEditorCmd : public IEvent
{
public:
    uint32_t                cmd;
};

class CEventDownloadEnd : public IEvent
{
public:
    enum DOWNLOAD_TYPE
    {
        DT_DL_LRC_FAILED,
        DT_DL_CHECK_NEW_VERSION_OK,
    };

    DOWNLOAD_TYPE        downloadType;
    class CDownloadTask    *pTask;

};

class CEventVisDrawUpdate : public IEvent
{
public:
    VisParam            *pVisParam;
};

class ISkinCmdHandler
{
public:
    ISkinCmdHandler() { m_pSkinWnd = nullptr; }
    virtual ~ISkinCmdHandler() { }

    virtual void init(CSkinWnd *pSkinWnd) { m_pSkinWnd = pSkinWnd; }

    // if the command id is processed, return true.
    virtual bool onCommand(int nId) = 0;
    virtual bool onCustomCommand(int nId) = 0;
    virtual bool onUIObjNotify(IUIObjNotify *pNotify) = 0;

    // IUICheckStatus method
    virtual bool getChecked(uint32_t nID, bool &bChecked) { return false; }
    virtual bool getRadioChecked(vector<uint32_t> &vIDs, uint32_t &nIDChecked) { return false; }

protected:
    CSkinWnd        *m_pSkinWnd;

};

class CMPEventsDispatcher : public CEventsDispatcher
{
public:
    CMPEventsDispatcher();

    virtual void quit();

    virtual void registerHandler(EventType eventType, IEventHandler *pHandler);
    virtual void unRegisterHandler(EventType eventType, IEventHandler *pHandler);

protected:
#ifdef _MPLAYER
    CMPVisAdapter            *m_pVisAdapter;
#endif

};

class CMPlayerSettings
{
public:
    // szValue: p1="v1" p2="v2"
    //    or        v1
    static void setLyricsDisplaySettings(cstr_t szSettingName, cstr_t szValue, bool bNotify = true)
    {
        setSettings(ET_LYRICS_DISPLAY_SETTINGS, SZ_SECT_LYR_DISPLAY, szSettingName, szValue, bNotify);
    }

    // szValue: p1="v1" p2="v2"
    //    or        v1
    static void setLyricsDisplaySettings(cstr_t szSettingName, int value, bool bNotify = true)
    {
        setSettings(ET_LYRICS_DISPLAY_SETTINGS, SZ_SECT_LYR_DISPLAY, szSettingName, value, bNotify);
    }

    //
    // Player UI
    //
    static void getPlayerUISettings(cstr_t szSettingName, string &strValue)
    {
        getSettings(SZ_SECT_UI, szSettingName, strValue);
    }

    // szValue: p1="v1" p2="v2"
    //    or        v1
    static void setPlayerUISettings(cstr_t szSettingName, cstr_t szValue, bool bNotify = true)
    {
        setSettings(ET_UI_SETTINGS_CHANGED, SZ_SECT_UI, szSettingName, szValue, bNotify);
    }

    // szValue: p1="v1" p2="v2"
    //    or        v1
    static void setPlayerUISettings(cstr_t szSettingName, int value, bool bNotify = true)
    {
        setSettings(ET_UI_SETTINGS_CHANGED, SZ_SECT_UI, szSettingName, value, bNotify);
    }

    static void getSettings(cstr_t szSectionName, cstr_t szSettingName, string &strValue)
    {
        strValue = g_profile.getString(szSectionName, szSettingName, "");
    }

    static int getSettings(cstr_t szSectionName, cstr_t szSettingName, int nValueDef)
    {
        return g_profile.getInt(szSectionName, szSettingName, nValueDef);
    }

    // szValue: p1="v1" p2="v2"
    //    or        v1
    static void setSettings(EventType eventType, cstr_t szSectionName, cstr_t szSettingName, cstr_t szValue, bool bNotify = true);

    static void setSettings(EventType eventType, cstr_t szSectionName, cstr_t szSettingName, int value, bool bNotify = true);

};

#endif // !defined(AFX_MPEVENTHOST_H__AE092490_ADF2_4326_9066_B35EE136A3DD__INCLUDED_)
