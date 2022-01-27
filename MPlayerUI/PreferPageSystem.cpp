#include "MPlayerApp.h"
#include "PreferPageSystem.h"
#include "MLProfile.h"


//////////////////////////////////////////////////////////////////////////

class CPagePfInternet : public CPagePfBase
{
    UIOBJECT_CLASS_NAME_DECLARE(CPagePfBase)
public:
    CPagePfInternet() : CPagePfBase(PAGE_INET, "CMD_SYSTEM_INTERNET")
    {
        m_bInitailed = false;
    }

    void onInitialUpdate()
    {
        CPagePfBase::onInitialUpdate();

        addOptBool(ET_NULL, SZ_SECT_UI, "CheckNewVer", true, "CID_C_CHECK_NEW_VERSION");
        
        initCheckButtons();
        //
        // HTTP 代理设置
        //
        string proxyServer;
        int        nProxyPort;
        bool    bUseProxy;

        bUseProxy = CMLProfile::inetGetProxy(proxyServer, nProxyPort);
        checkButton("CID_USE_PROXY", tobool(bUseProxy));

        // Proxy server address
        setUIObjectText("CID_HTTP_PROXY", g_profile.getString("ProxyServer", ""));
        setUIObjectText("CID_HTTP_PORT", g_profile.getString("ProxyPort", ""));

        // Proxy user name and password
        setUIObjectText("CID_USER", g_profile.getString("ProxyUser", ""));
        string strPwd = g_profile.encryptGetString("ProxyPassEnc", "");
        if (strPwd.empty())
            strPwd = g_profile.getString("ProxyPass", "");
        setUIObjectProperty("CID_PASS", "Style", "PASSWORD");
        setUIObjectText("CID_PASS", strPwd.c_str());

        m_bInitailed = true;
    }

    void onDestroy()
    {
        saveProxyServerUserPwd();

        CPagePfBase::onDestroy();
    }

    bool onCustomCommand(int nId)
    {
        if (nId == getIDByName("CID_LOAD_IE_PROXY"))
        {
            bool        bUseProxy;
            string        strSvr;
            int            nPort;
            if (loadProxySvrFromIE(bUseProxy, strSvr, nPort))
            {
                int        nHttpProxyType;

                checkButton("CID_USE_PROXY", tobool(bUseProxy));
                if (bUseProxy)
                    nHttpProxyType = HTTP_PROXY_OURS;
                else
                    nHttpProxyType = HTTP_PROXY_NONE;

                g_profile.writeInt("ProxyType", nHttpProxyType);

                setUIObjectText("CID_HTTP_PROXY", strSvr.c_str());
                setUIObjectText("CID_HTTP_PORT", CStrPrintf("%d", nPort).c_str());
            }
        }
        else if (nId == getIDByName("CID_USE_PROXY"))
        {
            int        nHttpProxyType;

            if (isButtonChecked(nId))
                nHttpProxyType = HTTP_PROXY_OURS;
            else
                nHttpProxyType = HTTP_PROXY_NONE;
            g_profile.writeInt("ProxyType", nHttpProxyType);
        }
        else
            return CPagePfBase::onCustomCommand(nId);

        return true;
    }

protected:
    void saveProxyServerUserPwd()
    {
        if (!m_bInitailed)
            return;

        g_profile.writeString("ProxyServer", 
            getUIObjectText("CID_HTTP_PROXY").c_str());

        g_profile.writeString("ProxyPort", 
            getUIObjectText("CID_HTTP_PORT").c_str());

        string strUser = getUIObjectText("CID_USER");
        g_profile.writeString("ProxyUser", strUser.c_str());

        string strPwd = getUIObjectText("CID_PASS");
        g_profile.encryptWriteString("ProxyPassEnc", strPwd.c_str());
        g_profile.writeString("ProxyPass", "");

        if (strUser.size())
        {
            CStrPrintf userPwd("%s:%s", strUser.c_str(), strPwd.c_str());
            string b64UserPwd = base64Encode((uint8_t *)userPwd.c_str(), userPwd.size());
            g_profile.writeString("Base64ProxyUserPass", b64UserPwd.c_str());
        }
    }

    bool        m_bInitailed;

};

UIOBJECT_CLASS_NAME_IMP(CPagePfInternet, "Container.Inet")

//////////////////////////////////////////////////////////////////////////

#ifndef _MPLAYER

#ifdef _MAC_OS

IdToString        _id2strPlayers[] = { { 0, "iTunes" },
    { 0, nullptr }
};

#else

IdToString        _id2strPlayers[] = { { 0, "Winamp2" }, { 0, "Winamp5" },
    { 0, "Windows Media Player" }, { 0, "iTunes" }, { 0, "Foobar2000" },
    { 0, "Quintessential Player" }, { 0, "Silverjuke" },
    { 0, "MediaMonkey" }, { 0, "KMPlayer" },
    { 0, "BSPlayer" }, { 0, "Media Jukebox" },
    { 0, "XMPlay" }, { 0, "AIMP2" }, { 0, "AIMP3" },
    { 0, "VLC Media Player" },
    { 0, nullptr }
};

#endif

int   _idWmp = 0;

#endif // #ifndef _MPLAYER

class CPagePfStartup: public CPagePfBase
{
    UIOBJECT_CLASS_NAME_DECLARE(CPagePfBase)
public:
    CPagePfStartup() : CPagePfBase(PAGE_STARTUP, "CMD_SYSTEM_STARTUP")
    {
        m_bInitailed = false;
    }

    void onInitialUpdate()
    {
        CPagePfBase::onInitialUpdate();

        m_bInitailed = true;

        addOptBool(ET_NULL, SZ_SECT_UI, "CheckNewVer", true, "CID_C_CHECK_NEW_VERSION");

        initCheckButtons();

#ifndef _MPLAYER
        CUIObject *pObjFrame = getUIObjectById("CID_STARTUP_FRAME", nullptr);
        assert(pObjFrame);
        if (!pObjFrame)
            return;

        CStrPrintf    strWidth("(%s)/2-10", pObjFrame->m_formWidth.getFormula());
        CStrPrintf    strP1Left("%s + 10", pObjFrame->m_formLeft.getFormula());
        CStrPrintf    strP2Left("%s+(%s)/2+10", pObjFrame->m_formLeft.getFormula(),
            pObjFrame->m_formWidth.getFormula());
        CFormula    formTop;

        formTop.setFormula(pObjFrame->m_formTop.getFormula());
        formTop.increase(25);

        for (int i = 0; _id2strPlayers[i].szId != nullptr; i++)
        {
            bool bLeft = (i % 2 == 0);

            if (_id2strPlayers[i].dwId == 0)
                _id2strPlayers[i].dwId = m_pSkin->getSkinFactory()->allocUID();

            if (strcmp(_id2strPlayers[i].szId, "Windows Media Player") == 0)
                _idWmp = _id2strPlayers[i].dwId;

            CSkinNStatusButton *pButton = (CSkinNStatusButton*)m_pSkin->getSkinFactory()->createDynamicCtrl(
                this, "NormalCheckBox", _id2strPlayers[i].dwId,
                bLeft ? strP1Left.c_str() : strP2Left.c_str(), formTop.getFormula(), strWidth.c_str());
            if (pButton)
            {
                pButton->setProperty(SZ_PN_TEXT, _id2strPlayers[i].szId);
                if (g_profile.getInt(_id2strPlayers[i].szId, true))
                    pButton->setStatus(true);
            }

            if (!bLeft)
                formTop.increase(30);
        }
#endif // #ifndef _MPLAYER
    }

    bool onCustomCommand(int nId)
    {
#ifndef _MPLAYER
        //
        // Should MiniLyrics get started with the checked player?
        //
        cstr_t        szPlayer;
        szPlayer = iDToString(_id2strPlayers, nId, "");
        if (!isEmptyString(szPlayer))
        {
            g_profile.writeInt(szPlayer, isButtonChecked(nId));
            return true;
        }
#endif // #ifndef _MPLAYER

        return CPagePfBase::onCustomCommand(nId);
    }

    void onDestroy()
    {
#ifdef _MINILYRICS_WIN32
        if (m_bInitailed)
        {
            assert(_idWmp != UID_INVALID);
            regWriteProfileInt(HKEY_CURRENT_USER, "Software\\Microsoft\\MediaPlayer\\UIPlugins\\{46B5EE7F-3B6B-4079-A756-5EFC10B1F50B}", 
                "Running", isButtonChecked(_idWmp));
        }
#endif // #ifdef _MINILYRICS_WIN32

        CPagePfBase::onDestroy();
    }

protected:
    bool        m_bInitailed;

};

UIOBJECT_CLASS_NAME_IMP(CPagePfStartup, "Container.Startup")

//////////////////////////////////////////////////////////////////////////

UIOBJECT_CLASS_NAME_IMP(CPagePfSystemRoot, "PreferPage.SystemRoot")

CPagePfSystemRoot::CPagePfSystemRoot() : CPagePfBase(PAGE_UNKNOWN, "CMD_ROOT_SYSTEM")
{
}

void CPagePfSystemRoot::onInitialUpdate()
{
    CPagePfBase::onInitialUpdate();

    checkToolbarDefaultPage("CID_TOOLBAR_SYSTEM");
}

void registerPfSystemPages(CSkinFactory *pSkinFactory)
{
    AddUIObjNewer2(pSkinFactory, CPagePfInternet);
    AddUIObjNewer2(pSkinFactory, CPagePfStartup);
    AddUIObjNewer2(pSkinFactory, CPagePfSystemRoot);
}

