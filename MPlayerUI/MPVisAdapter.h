// MPVisAdapter.h: interface for the CMPVisAdapter class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MPVISADAPTER_H__2B6D8B6F_84B1_415E_9026_69C1D96B48CF__INCLUDED_)
#define AFX_MPVISADAPTER_H__2B6D8B6F_84B1_415E_9026_69C1D96B48CF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CMPVisAdapter : public IVis
{
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

#endif // !defined(AFX_MPVISADAPTER_H__2B6D8B6F_84B1_415E_9026_69C1D96B48CF__INCLUDED_)
