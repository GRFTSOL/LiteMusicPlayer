// MPVisAdapter.cpp: implementation of the CMPVisAdapter class.
//
//////////////////////////////////////////////////////////////////////

#include "MPlayerAppBase.h"
#include "MPVisAdapter.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMPVisAdapter::CMPVisAdapter()
{
    OBJ_REFERENCE_INIT
}

CMPVisAdapter::~CMPVisAdapter()
{

}

MLRESULT CMPVisAdapter::init(IMPlayer *pPlayer)
{
    return ERR_OK;
}

MLRESULT CMPVisAdapter::quit()
{
    return ERR_OK;
}

int CMPVisAdapter::render(VisParam *visParam)
{
    CEventVisDrawUpdate        *pEvent = new CEventVisDrawUpdate;

    // memcpy(&m_visParam, visParam, sizeof(m_visParam));

    pEvent->pVisParam = visParam; // &m_visParam;
    pEvent->eventType = ET_VIS_DRAW_UPDATE;
    CMPlayerAppBase::getEventsDispatcher()->dispatchUnsyncEvent(pEvent);

    return ERR_OK;
}
