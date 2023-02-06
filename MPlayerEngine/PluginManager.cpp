#include "MPluginManager.h"
#include "../Skin/SkinTypes.h"


#ifdef _WIN32

class PlayerPlugin {
public:
    PlayerPlugin() { m_hModule = nullptr; m_QueryPluginIF = nullptr; }
    virtual ~PlayerPlugin() { if (m_hModule) FreeLibrary(m_hModule); }

    bool open(cstr_t szModule) {
        if (m_hModule) {
            FreeLibrary(m_hModule);
            m_QueryPluginIF = nullptr;
        }

        m_hModule = LoadLibrary(szModule);
        if (m_hModule == nullptr) {
            return false;
        }

        return attach(m_hModule);
    }

    bool attach(HMODULE hModule) {
        m_hModule = hModule;
        m_QueryPluginIF = (DHPlayerQueryPluginIF_t)GetProcAddress(m_hModule, SZ_FUNC_ZP_QUERY_PLUGIN_IF);
        return m_QueryPluginIF != nullptr;
    }

    void detach() { m_hModule = nullptr; m_QueryPluginIF = nullptr; }

    ResultCode queryPluginIF(int index, MPInterfaceType *interfaceType, const char **description, void **interfacePtr) {
        if (m_QueryPluginIF) {
            return m_QueryPluginIF(index, interfaceType, description, interfacePtr);
        } else {
            return ERR_FALSE;
        }
    }

protected:
    HMODULE                     m_hModule;
    DHPlayerQueryPluginIF_t   m_QueryPluginIF;

};

#else

class PlayerPlugin {
public:
    PlayerPlugin() { m_QueryPluginIF = nullptr; }
    virtual ~PlayerPlugin() { }

    bool open(cstr_t szModule) {
        return false;
    }

    void detach() { }

    ResultCode queryPluginIF(int index, MPInterfaceType *interfaceType, const char **description, void **interfacePtr) {
        if (m_QueryPluginIF) {
            return m_QueryPluginIF(index, interfaceType, description, interfacePtr);
        } else {
            return ERR_FALSE;
        }
    }

protected:
    DHPlayerQueryPluginIF_t     m_QueryPluginIF;

};

#endif

PluginManager::PluginManager(void) {
    OBJ_REFERENCE_INIT
}

PluginManager::~PluginManager(void) {
}

//
// Detect all the plugins and manage them.
// All plugins are in "Plugins" folder.
//
ResultCode PluginManager::detectPlugins() {

    //
    // register internal plugins
    //
    IMediaDecoder *pDecoder = nullptr;

#ifdef _MAC_OS
    pDecoder = new CoreAVPlayer;
    onInternalDecoderRegister(pDecoder);
    pDecoder->release();
#else
#ifdef _MPLAYER
    IMPlayer *player = nullptr;
    CMPlayer::getInstance(&player);

#ifdef _WIN32
    IVisualizer *pVis = new CVISDemo();
    pVis->init(player);
    player->registerVis(pVis);
#endif

    pDecoder = new CMDLibmad;
    onInternalDecoderRegister(pDecoder);
    pDecoder->release();

    pDecoder = new CMDWave;
    onInternalDecoderRegister(pDecoder);
    pDecoder->release();

    pDecoder = new CMDRow;
    onInternalDecoderRegister(pDecoder);
    pDecoder->release();
#else
    pDecoder = new CMDWmpCore;
    onInternalDecoderRegister(pDecoder);
    pDecoder->release();
#endif
#endif // #ifndef _MAC_OS

    string strPluginFolder = getAppResourceDir();
    strPluginFolder += "Plugins";
    dirStringAddSep(strPluginFolder);

    FileFind finder;
    if (!finder.openDir(strPluginFolder.c_str())) {
        return ERR_OK;
    }

    while (finder.findNext()) {
        if (finder.isCurDir() || !fileIsExtSame(finder.getCurName(), ".dll")) {
            continue;
        }

        string strPlugin = strPluginFolder;
        strPlugin += finder.getCurName();

        PlayerPlugin plugin;

        if (!plugin.open(strPlugin.c_str())) {
            continue;
        }

        for (int i = 0; ; i++) {
            MPInterfaceType ifType;
            const char *desc = nullptr;
            auto nRet = plugin.queryPluginIF(i, &ifType, &desc, nullptr);
            if (nRet != ERR_OK) {
                break;
            }

            switch (ifType) {
            case MPIT_INPUT_DECTOR:
                {
                    CMPAutoPtr<IMediaInput> input;

                    nRet = plugin.queryPluginIF(i, &ifType, nullptr, (void **)&input);
                    if (nRet != ERR_OK || ifType != MPIT_INPUT) {
                        break;
                    }
                }
                break;
            case MPIT_DECODE:
                {
                    CMPAutoPtr<IMediaDecoder> decoder;

                    nRet = plugin.queryPluginIF(i, &ifType, nullptr, (void **)&decoder);
                    if (nRet != ERR_OK || ifType != MPIT_DECODE) {
                        break;
                    }

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


ResultCode PluginManager::onInternalDecoderRegister(IMediaDecoder *pDecoder) {
    return registerDecoder(pDecoder, "", 0);
}


ResultCode PluginManager::newInput(cstr_t szMediaUrl, IMediaInput **ppInput) {
    ResultCode nRet;

    *ppInput = new CMILocalFile;

    nRet = (*ppInput)->open(szMediaUrl);
    if (nRet != ERR_OK) {
        delete *ppInput;
        *ppInput = nullptr;
        return nRet;
    }

    (*ppInput)->addRef();
    return ERR_NOT_SUPPORT;
}


ResultCode PluginManager::newDecoder(IMediaInput *pInput, IMediaDecoder **ppDecoder) {
    cstr_t szMediaUrl = pInput->getSource();
    ListDecodersInfo::iterator it;
    cstr_t szExt;

    *ppDecoder = nullptr;

    szExt = fileGetExt(szMediaUrl);
    if (isEmptyString(szExt)) {
        return ERR_NOT_SUPPORT_FILE_FORMAT;
    }

    string strExtLower = toLower(szExt);

    for (it = m_listDecoders.begin(); it != m_listDecoders.end(); ++it) {
        DecoderInfo &info = *it;

        if (find(info.vExt.begin(), info.vExt.end(), strExtLower) != info.vExt.end()) {
            PlayerPlugin plugin;
            MPInterfaceType mpit;

            if (!plugin.open(info.strFileName.c_str())) {
                break;    // Try next plugin
            }

            if (plugin.queryPluginIF(info.nIFIndex, &mpit, nullptr, (void * *)ppDecoder) != ERR_OK) {
                break;    // Try next plugin
            }
            plugin.detach();
            assert(mpit == MPIT_DECODE);

            return ERR_OK;
        }
    }

    return ERR_NOT_SUPPORT_FILE_FORMAT;
}


ResultCode PluginManager::newOutput(IMediaOutput **ppOutput) {
#ifndef _MAC_OS
    *ppOutput = new CMOSoundCard;
    return ERR_OK;
#else
    return ERR_NOT_SUPPORT;
#endif // #ifndef _MAC_OS
}


ResultCode PluginManager::getActiveDSP(IDSP **ppDSP) {
#ifdef _MPLAYER
    *ppDSP = CDspSuperEQ::getInstance();
    return ERR_OK;
#else
    return ERR_NOT_SUPPORT;
#endif
}


VecVisualizers PluginManager::getActiveVis() {
    return VecVisualizers();
}

ResultCode PluginManager::registerDecoder(IMediaDecoder *pDecoder, cstr_t szModuleFileName, int nIFIndex) {
    DecoderInfo decoderInfo;

    decoderInfo.desc = pDecoder->getDescription();
    decoderInfo.strFileName = szModuleFileName;
    decoderInfo.nIFIndex = nIFIndex;

    // Is this decoder registered?
    for (DecoderInfo &info : m_listDecoders) {
        if (!info.strFileName.empty()
            && strcasecmp(info.strFileName.c_str(), szModuleFileName) == 0
            && info.nIFIndex == nIFIndex) {
            info = decoderInfo;
            return true;
        }
    }

    vector<string> vStrExt;
    strSplit(pDecoder->getFileExtentions(), '|', vStrExt);
    for (int i = 0; i < vStrExt.size() - 1; i+= 2) {
        if (vStrExt[i].empty()) {
            continue;
        }

        // Ext is in lower case, for later compare.
        decoderInfo.vExt.push_back(toLower(vStrExt[i].c_str()));
        decoderInfo.vExtDesc.push_back(vStrExt[i + 1]);
    }

    m_listDecoders.push_back(decoderInfo);

    return true;
}


void PluginManager::unregisterDecoder(cstr_t szModuleFileName) {
    ListDecodersInfo::iterator it, itEnd;

    itEnd = m_listDecoders.end();
    for (it = m_listDecoders.begin(); it != itEnd; ++it) {
        DecoderInfo &item = *it;
        if (strcmp(item.strFileName.c_str(), szModuleFileName) == 0) {
            m_listDecoders.erase(it);
            return;
        }
    }
}
