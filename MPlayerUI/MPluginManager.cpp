#include "Player.h"
#include "MPluginManager.h"
#include "../Skin/SkinTypes.h"


#ifdef _WIN32

class CZikiPlayerPlugin
{
public:
    CZikiPlayerPlugin() { m_hModule = nullptr; m_QueryPluginIF = nullptr; }
    virtual ~CZikiPlayerPlugin() { if (m_hModule) FreeLibrary(m_hModule); }

    bool open(cstr_t szModule)
    {
        if (m_hModule)
        {
            FreeLibrary(m_hModule);
            m_QueryPluginIF = nullptr;
        }

        m_hModule = LoadLibrary(szModule);
        if (m_hModule == nullptr)
            return false;

        return attach(m_hModule);
    }

    bool attach(HMODULE hModule)
    {
        m_hModule = hModule;
        m_QueryPluginIF = (ZikiPlayerQueryPluginIF_t)GetProcAddress(m_hModule, SZ_FUNC_ZP_QUERY_PLUGIN_IF);
        return m_QueryPluginIF != nullptr;
    }

    void detach() { m_hModule = nullptr; m_QueryPluginIF = nullptr; }

    MLRESULT zikiPlayerQueryPluginIF(
        int nIndex,
        MPInterfaceType *pInterfaceType,
        IXStr *strDescription,
        void * *lpInterface
        )
    {
        if (m_QueryPluginIF)
            return m_QueryPluginIF(nIndex, pInterfaceType, strDescription, lpInterface);
        else
            return ERR_FALSE;
    }

protected:
    HMODULE                        m_hModule;
    ZikiPlayerQueryPluginIF_t    m_QueryPluginIF;

};

#else

class CZikiPlayerPlugin
{
public:
    CZikiPlayerPlugin() { m_QueryPluginIF = nullptr; }
    virtual ~CZikiPlayerPlugin() { }

    bool open(cstr_t szModule)
    {
        return false;
    }

    void detach() { }

    MLRESULT zikiPlayerQueryPluginIF(
        int nIndex,
        MPInterfaceType *pInterfaceType,
        IXStr *strDescription,
        void * *lpInterface
        )
    {
        if (m_QueryPluginIF)
            return m_QueryPluginIF(nIndex, pInterfaceType, strDescription, lpInterface);
        else
            return ERR_FALSE;
    }

protected:
    ZikiPlayerQueryPluginIF_t    m_QueryPluginIF;

};

#endif

CMPluginManager::CMPluginManager(void)
{
    OBJ_REFERENCE_INIT
}


CMPluginManager::~CMPluginManager(void)
{
}


//
// Detect all the plugins and manage them.
// All plugins are in "Plugins" folder.
//
MLRESULT CMPluginManager::detectPlugins()
{
    string            strPluginFolder, strPlugin;
    FileFind        finder;

    strPluginFolder = getAppResourceDir();
    strPluginFolder += "Plugins";
    dirStringAddSlash(strPluginFolder);

    if (!finder.openDir(strPluginFolder.c_str()))
        return ERR_OK;

    while (finder.findNext())
    {
        if (finder.isCurDir() || !fileIsExtSame(finder.getCurName(), ".dll"))
            continue;

        strPlugin = strPluginFolder;
        strPlugin += finder.getCurName();

        CZikiPlayerPlugin        plugin;

        if (!plugin.open(strPlugin.c_str()))
            continue;

        for (int i = 0; ; i++)
        {
            MPInterfaceType    ifType;
            MLRESULT        nRet;
            CXStr            desc;

            nRet = plugin.zikiPlayerQueryPluginIF(i, &ifType, &desc, nullptr);
            if (nRet != ERR_OK)
                break;

            switch (ifType)
            {
            case MPIT_INPUT_DECTOR:
                {
                    CMPAutoPtr<IMediaInput>    input;

                    nRet = plugin.zikiPlayerQueryPluginIF(i, &ifType, nullptr, (void **)&input);
                    if (nRet != ERR_OK || ifType != MPIT_INPUT)
                        break;
                }
                break;
            case MPIT_DECODE:
                {
                    CMPAutoPtr<IMediaDecode>    decoder;

                    nRet = plugin.zikiPlayerQueryPluginIF(i, &ifType, nullptr, (void **)&decoder);
                    if (nRet != ERR_OK || ifType != MPIT_DECODE)
                        break;

                    registerDecoder(decoder, strPlugin.c_str(), i);
                }
                break;
            default:
                break;
            }
        }
    }

    return ERR_OK;
}


MLRESULT CMPluginManager::onInternalDecoderRegister(IMediaDecode *pDecoder)
{
    return registerDecoder(pDecoder, "", 0);
}


MLRESULT CMPluginManager::newInput(LPCXSTR szMediaUrl, IMediaInput **ppInput)
{
    // Use internal input
    return ERR_NOT_SUPPORT;
}


MLRESULT CMPluginManager::newDecoder(IMediaInput *pInput, IMediaDecode **ppDecoder)
{
    LPCXSTR                        szMediaUrl = pInput->getSource();
    ListDecodersInfo::iterator    it;
    cstr_t                        szExt;

    *ppDecoder = nullptr;

    szExt = fileGetExt(szMediaUrl);
    if (isEmptyString(szExt))
        return ERR_NOT_SUPPORT_FILE_FORMAT;

    string strExtLower = toLower(szExt);

    for (it = m_listDecoders.begin(); it != m_listDecoders.end(); ++it)
    {
        DecoderInfo        &info = *it;

        if (find(info.vExt.begin(), info.vExt.end(), strExtLower) != info.vExt.end())
        {
            CZikiPlayerPlugin    plugin;
            MPInterfaceType        mpit;

            if (!plugin.open(info.strFileName.c_str()))
                break;    // Try next plugin

            if (plugin.zikiPlayerQueryPluginIF(info.nIFIndex, &mpit, nullptr, (void * *)ppDecoder) != ERR_OK)
                break;    // Try next plugin
            plugin.detach();
            assert(mpit == MPIT_DECODE);

            return ERR_OK;
        }
    }

    return ERR_NOT_SUPPORT_FILE_FORMAT;
}


MLRESULT CMPluginManager::newOutput(IMediaOutput **ppOutput)
{
    // Use internal output
    return ERR_NOT_SUPPORT;
}


MLRESULT CMPluginManager::getActiveDSP(IDSP **ppDSP)
{
    return ERR_NOT_SUPPORT;
}


MLRESULT CMPluginManager::getActiveVis(IVector *pvVis)
{
    return ERR_NOT_SUPPORT;
}

MLRESULT CMPluginManager::registerDecoder(IMediaDecode *pDecoder, cstr_t szModuleFileName, int nIFIndex)
{
    DecoderInfo        decoderInfo;

    decoderInfo.desc = pDecoder->getDescription();
    decoderInfo.strFileName = szModuleFileName;
    decoderInfo.nIFIndex = nIFIndex;

    // Is this decoder registered?
    for (ListDecodersInfo::iterator it = m_listDecoders.begin(); it != m_listDecoders.end(); ++it)
    {
        DecoderInfo        &info = *it;
        if (!info.strFileName.empty()
            && strcasecmp(info.strFileName.c_str(), szModuleFileName) == 0
            && info.nIFIndex == nIFIndex)
        {
            info = decoderInfo;
            return true;
        }
    }

    vector<string>    vStrExt;
    strSplit(pDecoder->getFileExtentions(), '|', vStrExt);
    for (int i = 0; i < vStrExt.size() - 1; i+= 2)
    {
        if (vStrExt[i].empty())
            continue;

        // Ext is in lower case, for later compare.
        decoderInfo.vExt.push_back(toLower(vStrExt[i].c_str()));
        decoderInfo.vExtDesc.push_back(vStrExt[i + 1]);
    }

    m_listDecoders.push_back(decoderInfo);

    return true;
}


void CMPluginManager::unregisterDecoder(cstr_t szModuleFileName)
{
    ListDecodersInfo::iterator        it, itEnd;

    itEnd = m_listDecoders.end();
    for (it = m_listDecoders.begin(); it != itEnd; ++it)
    {
        DecoderInfo    &item = *it;
        if (strcmp(item.strFileName.c_str(), szModuleFileName) == 0)
        {
            m_listDecoders.erase(it);
            return;
        }
    }
}
