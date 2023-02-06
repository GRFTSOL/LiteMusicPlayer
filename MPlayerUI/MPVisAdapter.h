#pragma once

#ifndef MPlayerUI_MPVisAdapter_h
#define MPlayerUI_MPVisAdapter_h


class CMPVisAdapter : public IVisualizer {
    OBJ_REFERENCE_DECL
public:
    CMPVisAdapter();
    virtual ~CMPVisAdapter();

    virtual ResultCode init(IMPlayer *pPlayer);
    virtual ResultCode quit();

    virtual int render(VisParam *visParam);

protected:
    // VisParam        m_visParam;

};

#endif // !defined(MPlayerUI_MPVisAdapter_h)
