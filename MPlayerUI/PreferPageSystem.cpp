#include "MPlayerApp.h"
#include "PreferPageSystem.h"
#include "MLProfile.h"


//////////////////////////////////////////////////////////////////////////

class CPagePfInternet : public CPagePfBase {
    UIOBJECT_CLASS_NAME_DECLARE(CPagePfBase)
public:
    CPagePfInternet() : CPagePfBase(PAGE_INET, "ID_SYSTEM_INTERNET") {
        m_bInitailed = false;
    }

    void onInitialUpdate() override {
        CPagePfBase::onInitialUpdate();

        addOptBool(ET_NULL, SZ_SECT_UI, "CheckNewVer", true, "CID_C_CHECK_NEW_VERSION");

        initCheckButtons();
        //
        // HTTP 代理设置
        //
        string proxyServer;
        int nProxyPort;
        bool bUseProxy;

        bUseProxy = CMLProfile::inetGetProxy(proxyServer, nProxyPort);
        checkButton("CID_USE_PROXY", tobool(bUseProxy));

        // Proxy server address
        setUIObjectText("CID_HTTP_PROXY", g_profile.getString("ProxyServer", ""));
        setUIObjectText("CID_HTTP_PORT", g_profile.getString("ProxyPort", ""));

        // Proxy user name and password
        setUIObjectText("CID_USER", g_profile.getString("ProxyUser", ""));
        string strPwd = g_profile.encryptGetString("ProxyPassEnc", "");
        if (strPwd.empty()) {
            strPwd = g_profile.getString("ProxyPass", "");
        }
        setUIObjectProperty("CID_PASS", "Style", "PASSWORD");
        setUIObjectText("CID_PASS", strPwd.c_str());

        m_bInitailed = true;
    }

    void onDestroy() override {
        saveProxyServerUserPwd();

        CPagePfBase::onDestroy();
    }

    bool onCommand(uint32_t nId) override {
        if (nId == getIDByName("CID_LOAD_IE_PROXY")) {
            bool bUseProxy;
            string strSvr;
            int nPort;
            if (loadProxySvrFromIE(bUseProxy, strSvr, nPort)) {
                int nHttpProxyType;

                checkButton("CID_USE_PROXY", tobool(bUseProxy));
                if (bUseProxy) {
                    nHttpProxyType = HTTP_PROXY_OURS;
                } else {
                    nHttpProxyType = HTTP_PROXY_NONE;
                }

                g_profile.writeInt("ProxyType", nHttpProxyType);

                setUIObjectText("CID_HTTP_PROXY", strSvr.c_str());
                setUIObjectText("CID_HTTP_PORT", stringPrintf("%d", nPort).c_str());
            }
        } else if (nId == getIDByName("CID_USE_PROXY")) {
            int nHttpProxyType;

            if (isButtonChecked(nId)) {
                nHttpProxyType = HTTP_PROXY_OURS;
            } else {
                nHttpProxyType = HTTP_PROXY_NONE;
            }
            g_profile.writeInt("ProxyType", nHttpProxyType);
        } else {
            return CPagePfBase::onCommand(nId);
        }

        return true;
    }

protected:
    void saveProxyServerUserPwd() {
        if (!m_bInitailed) {
            return;
        }

        g_profile.writeString("ProxyServer",
            getUIObjectText("CID_HTTP_PROXY").c_str());

        g_profile.writeString("ProxyPort",
            getUIObjectText("CID_HTTP_PORT").c_str());

        string strUser = getUIObjectText("CID_USER");
        g_profile.writeString("ProxyUser", strUser.c_str());

        string strPwd = getUIObjectText("CID_PASS");
        g_profile.encryptWriteString("ProxyPassEnc", strPwd.c_str());
        g_profile.writeString("ProxyPass", "");

        if (strUser.size()) {
            string userPwd = stringPrintf("%s:%s", strUser.c_str(), strPwd.c_str());
            string b64UserPwd = base64Encode((uint8_t *)userPwd.c_str(), userPwd.size());
            g_profile.writeString("Base64ProxyUserPass", b64UserPwd.c_str());
        }
    }

    bool                        m_bInitailed;

};

UIOBJECT_CLASS_NAME_IMP(CPagePfInternet, "Container.Inet")

//////////////////////////////////////////////////////////////////////////

class CPagePfStartup: public CPagePfBase {
    UIOBJECT_CLASS_NAME_DECLARE(CPagePfBase)
public:
    CPagePfStartup() : CPagePfBase(PAGE_STARTUP, "ID_SYSTEM_STARTUP") {
        m_bInitailed = false;
    }

    void onInitialUpdate() override {
        CPagePfBase::onInitialUpdate();

        m_bInitailed = true;

        addOptBool(ET_NULL, SZ_SECT_UI, "CheckNewVer", true, "CID_C_CHECK_NEW_VERSION");

        initCheckButtons();
    }

    void onDestroy() override {
        CPagePfBase::onDestroy();
    }

protected:
    bool                        m_bInitailed;

};

UIOBJECT_CLASS_NAME_IMP(CPagePfStartup, "Container.Startup")

//////////////////////////////////////////////////////////////////////////

UIOBJECT_CLASS_NAME_IMP(CPagePfSystemRoot, "PreferPage.SystemRoot")

CPagePfSystemRoot::CPagePfSystemRoot() : CPagePfBase(PAGE_UNKNOWN, "ID_ROOT_SYSTEM") {
}

void CPagePfSystemRoot::onInitialUpdate() {
    CPagePfBase::onInitialUpdate();

    checkToolbarDefaultPage("CID_TOOLBAR_SYSTEM");
}

void registerPfSystemPages(CSkinFactory *pSkinFactory) {
    AddUIObjNewer2(pSkinFactory, CPagePfInternet);
    AddUIObjNewer2(pSkinFactory, CPagePfStartup);
    AddUIObjNewer2(pSkinFactory, CPagePfSystemRoot);
}
