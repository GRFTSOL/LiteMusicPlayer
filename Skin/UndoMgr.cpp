#include "SkinTypes.h"
#include "UndoMgr.h"


#define MAX_ACTIONS            200

//////////////////////////////////////////////////////////////////////////
// CBatchUndoAction
CBatchUndoAction::CBatchUndoAction()
{

}

CBatchUndoAction::~CBatchUndoAction()
{
    ListActions::iterator it, itEnd;
    itEnd = m_listActions.end();
    for (it = m_listActions.begin(); it != itEnd; ++it)
    {
        delete *it;
    }
    m_listActions.clear();
}

void CBatchUndoAction::undo()
{
    ListActions::reverse_iterator it, itEnd;
    itEnd = m_listActions.rend();
    for (it = m_listActions.rbegin(); it != itEnd; ++it)
    {
        (*it)->undo();
    }
}

void CBatchUndoAction::redo()
{
    ListActions::iterator it, itEnd;
    itEnd = m_listActions.end();
    for (it = m_listActions.begin(); it != itEnd; ++it)
    {
        (*it)->redo();
    }
}

void CBatchUndoAction::addAction(CUndoAction *pAction)
{
    m_listActions.push_back(pAction);
}

bool CBatchUndoAction::isEmpty()
{
    return m_listActions.empty();
}

//////////////////////////////////////////////////////////////////////////

CUndoMgr::CUndoMgr()
{
    m_posUndo = -1;
    m_pBatchUndoAction = nullptr;
    m_pNotify = nullptr;

    m_bCanUndoPrev = false;
    m_bCanRedoPrev = false;
}


CUndoMgr::~CUndoMgr()
{
    if (m_pBatchUndoAction)
    {
        delete m_pBatchUndoAction;
        m_pBatchUndoAction = nullptr;
    }
    clear();
}


void CUndoMgr::beginBatchAction()
{
    assert(!m_pBatchUndoAction);
    if (m_pBatchUndoAction)
    {
        endBatchAction();
    }
    m_pBatchUndoAction = new CBatchUndoAction;
}


void CUndoMgr::endBatchAction()
{
    assert(m_pBatchUndoAction);
    if (m_pBatchUndoAction)
    {
        if (m_pBatchUndoAction->isEmpty())
            delete m_pBatchUndoAction;
        else
            doAddAction(m_pBatchUndoAction);
        m_pBatchUndoAction = nullptr;
    }
}


void CUndoMgr::addAction(CUndoAction *pAction)
{
    if (m_pBatchUndoAction)
        m_pBatchUndoAction->addAction(pAction);
    else
        doAddAction(pAction);
}


bool CUndoMgr::undo()
{
    assert(!m_pBatchUndoAction);
    if (m_pBatchUndoAction)
        endBatchAction();

    assert(m_posUndo >= -1 && m_posUndo < (int)m_listActions.size());
    if (canUndo())
    {
        m_listActions[m_posUndo]->undo();
        m_posUndo--;

        if (m_pNotify)
            m_pNotify->onAction(IUndoMgrNotify::A_UNDO);

        onDetectRedoStatus();
        onDetectUndoStatus();
        return true;
    }
    return false;
}


bool CUndoMgr::redo()
{
    assert(!m_pBatchUndoAction);
    if (m_pBatchUndoAction)
        endBatchAction();

    // assert(canRedo);
    if (canRedo())
    {
        m_listActions[m_posUndo + 1]->redo();
        m_posUndo++;

        if (m_pNotify)
            m_pNotify->onAction(IUndoMgrNotify::A_REDO);

        onDetectRedoStatus();
        onDetectUndoStatus();
        return true;
    }
    return false;
}


bool CUndoMgr::canUndo()
{
    return m_posUndo >= 0 && m_listActions.size();
}


bool CUndoMgr::canRedo()
{
    return m_listActions.size() && m_posUndo < int(m_listActions.size() - 1);
}


void CUndoMgr::clear()
{
    assert(!m_pBatchUndoAction);
    if (m_pBatchUndoAction)
        endBatchAction();

    if (m_listActions.size())
    {
        onDetectRedoStatus();
        onDetectUndoStatus();
    }

    for (ListActions::iterator it = m_listActions.begin(); it != m_listActions.end(); it++)
    {
        delete *it;
    }
    m_listActions.clear();

    m_posUndo = -1;
}


void CUndoMgr::onDetectUndoStatus()
{
    bool        bNew = canUndo();
    if (m_bCanUndoPrev != bNew)
    {
        m_bCanUndoPrev = bNew;
        if (m_pNotify)
            m_pNotify->onStatusChanged(IUndoMgrNotify::S_CAN_UNDO, bNew);
    }
}


void CUndoMgr::onDetectRedoStatus()
{
    bool        bNew = canRedo();
    if (m_bCanRedoPrev != bNew)
    {
        m_bCanRedoPrev = bNew;
        if (m_pNotify)
            m_pNotify->onStatusChanged(IUndoMgrNotify::S_CAN_REDO, bNew);
    }
}


void CUndoMgr::doAddAction(CUndoAction *pAction)
{
    assert(m_posUndo >= -1);
    if (m_posUndo < (int)(m_listActions.size() - 1))
    {
        for (ListActions::iterator it = m_listActions.begin() + (m_posUndo + 1);
            it != m_listActions.end(); it++)
        {
            delete *it;
        }
        m_listActions.erase(m_listActions.begin() + (m_posUndo + 1), m_listActions.end());
    }

    if (m_listActions.size() > MAX_ACTIONS)
        m_listActions.erase(m_listActions.begin());

    m_listActions.push_back(pAction);
    m_posUndo = m_listActions.size() - 1;

    if (m_pNotify)
        m_pNotify->onAction(IUndoMgrNotify::A_ADD);

    onDetectRedoStatus();
    onDetectUndoStatus();
}
