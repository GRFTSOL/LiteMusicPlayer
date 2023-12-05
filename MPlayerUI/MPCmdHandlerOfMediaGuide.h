#pragma once

#include "MPMediaTree.h"


class CMPCmdHandlerOfMediaGuide : public ISkinCmdHandler {
public:
    CMPCmdHandlerOfMediaGuide();
    virtual ~CMPCmdHandlerOfMediaGuide();

    virtual void init(CSkinWnd *pSkinWnd);

    // if the command id is processed, return true.
    virtual bool onCommand(uint32_t nId);
    virtual bool onUIObjNotify(IUIObjNotify *pNotify);

protected:
    void onDblClickMediaList();

    void reloadMediaGuideView();
    void updateMediaList();

    void addHistoryPath();
    void backHistoryPath();

protected:
    struct HistroyItem {
        SkinTreeStrPath_t           path;
        int                         nSelChild;
    };

    CMPMediaTree                m_mediaTree;
    SkinTreeStrPath_t           m_vPathLatest;
    list<HistroyItem>           m_historyPath;

};
