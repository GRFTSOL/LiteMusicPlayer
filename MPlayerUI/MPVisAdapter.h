#pragma once

#ifndef MPlayerUI_MPVisAdapter_h
#define MPlayerUI_MPVisAdapter_h


class CMPVisAdapter : public IVis {
    OBJ_REFERENCE_DECL
public:
    CMPVisAdapter();
    virtual ~CMPVisAdapter();

    virtual MLRESULT init(IMPlayer *pPlayer);
    virtual MLRESULT quit();

    virtual int render(VisParam *visParam);

protected:
    // VisParam        m_visParam;

};

#endif // !defined(MPlayerUI_MPVisAdapter_h)
