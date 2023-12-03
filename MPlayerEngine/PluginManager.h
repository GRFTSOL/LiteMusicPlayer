#pragma once

#include "../MPlayerEngine/IMPlayer.h"


struct DecoderInfo {
    VecStrings                  vExt, vExtDesc;     // Supported file extension and descriptions.
    string                      desc;
    string                      strFileName;
    int                         nIFIndex;           // index in zikiPlayerQueryPluginIF
};

typedef list<DecoderInfo> ListDecodersInfo;
typedef vector<IVisualizer *> VecVisualizers;

class PluginManager {
public:
    PluginManager(void);
    virtual ~PluginManager(void);

    virtual ResultCode detectPlugins();

    virtual ResultCode onInternalDecoderRegister(IMediaDecoder *pDecoder);

    virtual ResultCode newInput(cstr_t szMediaUrl, IMediaInput **ppInput);
    virtual ResultCode newDecoder(IMediaInput *pInput, IMediaDecoder **ppDecoder);
    virtual ResultCode newOutput(IMediaOutput **ppOutput);

    virtual ResultCode getActiveDSP(IDSP **ppDSP);
    virtual VecVisualizers getActiveVis();

public:
    const ListDecodersInfo &getDecodersInfo() const { return m_listDecoders; }

    // decoders
    ResultCode registerDecoder(IMediaDecoder *pDecoder, cstr_t szModuleFileName, int nIFIndex);
    void unregisterDecoder(cstr_t szModuleFileName);

protected:
    // all decoders that MPlayer loaded.
    ListDecodersInfo            m_listDecoders;

};
