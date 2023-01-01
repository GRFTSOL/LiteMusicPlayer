#pragma once

#include "IMPlayer.h"
#include "MPTools.h"
#include "Media.h"


class CMPlayer;

class CPlaylist : public IPlaylist {
    OBJ_REFERENCE_DECL
public:
    CPlaylist(CMPlayer *pPlayer);
    virtual ~CPlaylist();

    virtual uint32_t getCount();

    virtual MLRESULT getItem(int nIndex, IMedia **ppMedia);

    virtual MLRESULT getName(IString *str);

    virtual MLRESULT insertItem(int nIndex, IMedia *pMedia);

    virtual MLRESULT moveItem(int nIndexOld, int nIndexNew);

    virtual MLRESULT removeItem(int nIndex);

    virtual MLRESULT clear();

public:
    MLRESULT getItemIndex(IMedia *pMedia, int &nIndex);

protected:
    typedef vector<CMedia *>        V_MEDIA;

    std::mutex                  m_mutexDataAccess;
    V_MEDIA                     m_vMedia;

    CMPlayer                    *m_pPlayer;

};
