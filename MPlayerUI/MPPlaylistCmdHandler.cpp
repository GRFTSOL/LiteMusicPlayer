#include "MPlayerAppBase.h"
#include "MPPlaylistCmdHandler.h"
#include "MPHelper.h"
#include "DlgMediaInfo.h"
#include "MPlaylistCtrl.h"
#include "DlgNewPlaylist.hpp"


CMPPlaylistCmdHandler::CMPPlaylistCmdHandler() {

}

CMPPlaylistCmdHandler::~CMPPlaylistCmdHandler() {

}

// if the command id is processed, return true.
bool CMPPlaylistCmdHandler::onCommand(int nId) {
    switch (nId) {
    case IDC_OPEN_PL:
        showOpenPlaylistDialog(m_pSkinWnd);
        break;
    case IDC_CLEAR:
        g_player.clearNowPlaying();
        break;
    default:
        return false;
    }

    return true;
}

bool CMPPlaylistCmdHandler::onCustomCommand(int nID) {
    switch (nID) {
    case CMD_PL_NEW:
        showNewPlaylistDialog(m_pSkinWnd, g_player.newPlaylist());
        break;
    case CMD_PL_SAVE:
        showSavePlaylistDialog(m_pSkinWnd, g_player.getCurrentPlaylist());
        break;
    case CMD_PL_DEL:
        {
            CMPlaylistCtrl *plCtrl = (CMPlaylistCtrl*)m_pSkinWnd->getUIObjectByClassName(CMPlaylistCtrl::className());
            if (!plCtrl) {
                break;
            }
            plCtrl->deleteSelectedItems();
        }
        break;
    case CMD_PL_UP:
    case CMD_PL_DOWN:
        {
            CMPlaylistCtrl *plCtrl = (CMPlaylistCtrl*)m_pSkinWnd->getUIObjectByClassName(CMPlaylistCtrl::className());
            if (!plCtrl) {
                break;
            }

            plCtrl->offsetAllSelectedItems(nID == CMD_PL_DOWN);
        }
        break;
    case CMD_PL_PROPERTY:
        {
            MediaPtr media;
            CMPlaylistCtrl *plCtrl = (CMPlaylistCtrl*)m_pSkinWnd->getUIObjectByClassName(CMPlaylistCtrl::className());
            if (plCtrl && plCtrl->isVisible() && plCtrl->isParentVisible()) {
                int nSel;
                nSel = plCtrl->getNextSelectedItem(-1);
                if (nSel == -1) {
                    break;
                }

                auto playlist = g_player.getCurrentPlaylist();
                media = playlist->getItem(nSel);
            } else {
                media = g_player.getCurrentMedia();
            }

            showMediaInfoDialog(m_pSkinWnd, media);
        }
        break;
    default:
        return false;
    }

    return true;
}

bool CMPPlaylistCmdHandler::onUIObjNotify(IUIObjNotify *pNotify) {
    return false;
}
