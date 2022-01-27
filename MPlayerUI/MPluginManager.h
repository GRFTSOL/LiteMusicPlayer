#pragma once

#include "../MPlayerEngine/IMPlayer.h"

struct DecoderInfo
{
    VecStrings        vExt, vExtDesc;        // Supported file extension and descriptions.
    string        desc;
    string        strFileName;
    int            nIFIndex;            // index in zikiPlayerQueryPluginIF
};

typedef list<DecoderInfo>    ListDecodersInfo;

class CMPluginManager : public IMPluginManager
{
    OBJ_REFERENCE_DECL
public:
    CMPluginManager(void);
    virtual ~CMPluginManager(void);

    virtual MLRESULT detectPlugins();

    virtual MLRESULT onInternalDecoderRegister(IMediaDecode *pDecoder);

    virtual MLRESULT newInput(LPCXSTR szMediaUrl, IMediaInput **ppInput);
    virtual MLRESULT newDecoder(IMediaInput *pInput, IMediaDecode **ppDecoder);
    virtual MLRESULT newOutput(IMediaOutput **ppOutput);

    virtual MLRESULT getActiveDSP(IDSP **ppDSP);
    virtual MLRESULT getActiveVis(IVector *pvVis);

public:
    const ListDecodersInfo &getDecodersInfo() const { return m_listDecoders; }

    // decoders
    MLRESULT registerDecoder(IMediaDecode *pDecoder, cstr_t szModuleFileName, int nIFIndex);
    void unregisterDecoder(cstr_t szModuleFileName);

protected:
    // all decoders that MPlayer loaded.
    ListDecodersInfo        m_listDecoders;

};
