#ifndef _UNDOMGR_H__
#define _UNDOMGR_H__

#pragma once


class CUndoAction {
public:
    CUndoAction() {}
    virtual ~CUndoAction() {}
    virtual void undo() = 0;
    virtual void redo() = 0;

};


class CBatchUndoAction : public CUndoAction {
public:
    typedef vector<CUndoAction*>        ListActions;

    CBatchUndoAction();
    virtual ~CBatchUndoAction();

    virtual void undo();
    virtual void redo();
    virtual void addAction(CUndoAction *pAction);

    bool isEmpty();

protected:
    ListActions                 m_listActions;

};


class IUndoMgrNotify {
public:
    enum Status {
        S_CAN_UNDO,
        S_CAN_REDO,
    };

    enum Action {
        A_UNDO,
        A_REDO,
        A_ADD,
    };

    virtual void onStatusChanged(Status status, bool bVal) = 0;
    virtual void onAction(Action action) = 0;

};


//
// Common undo and redo manager
//
class CUndoMgr {
public:
    typedef vector<CUndoAction*>        ListActions;

    CUndoMgr();
    ~CUndoMgr();

    void setNotification(IUndoMgrNotify *pNotify) { m_pNotify = pNotify; }

    void beginBatchAction(CBatchUndoAction *batchAction = nullptr);
    void endBatchAction();
    bool isInBatchAction() const { return m_pBatchUndoAction != nullptr; }

    virtual void addAction(CUndoAction *pAction);

    virtual bool undo();
    virtual bool redo();

    virtual bool canUndo();
    virtual bool canRedo();

    virtual void clear();

protected:
    void onDetectUndoStatus();
    void onDetectRedoStatus();

    void doAddAction(CUndoAction *pAction);

protected:
    ListActions                 m_listActions;
    int                         m_posUndo;
    IUndoMgrNotify              *m_pNotify;

    CBatchUndoAction            *m_pBatchUndoAction;

    bool                        m_bCanUndoPrev, m_bCanRedoPrev;

};


#endif // _UNDOMGR_H__
