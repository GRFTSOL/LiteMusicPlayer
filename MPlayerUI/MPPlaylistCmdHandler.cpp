#include "MPlayerAppBase.h"
#include "MPPlaylistCmdHandler.h"
#include "MPHelper.h"
#include "DlgMediaInfo.h"
#include "MPlaylistCtrl.h"
#include "DlgPlaylist.hpp"


CMPPlaylistCmdHandler::CMPPlaylistCmdHandler() {

}

CMPPlaylistCmdHandler::~CMPPlaylistCmdHandler() {

}

// if the command id is processed, return true.
bool CMPPlaylistCmdHandler::onCommand(uint32_t nId) {
    switch (nId) {
    case ID_OPEN_PL:
        showOpenPlaylistDialog(m_pSkinWnd);
        break;
    case ID_CLEAR:
        g_player.clearNowPlaying();
        break;
    case ID_PL_NEW:
        showNewPlaylistDialog(m_pSkinWnd, g_player.newPlaylist());
        break;
    case ID_PL_SAVE:
        showSavePlaylistDialog(m_pSkinWnd, g_player.getNowPlaying());
        break;
    case ID_PL_DEL:
        {
            CMPlaylistCtrl *plCtrl = (CMPlaylistCtrl*)m_pSkinWnd->getUIObjectByClassName(CMPlaylistCtrl::className());
            if (!plCtrl) {
                break;
            }
            plCtrl->deleteSelectedItems();
        }
        break;
    case ID_PL_UP:
    case ID_PL_DOWN:
        {
            CMPlaylistCtrl *plCtrl = (CMPlaylistCtrl*)m_pSkinWnd->getUIObjectByClassName(CMPlaylistCtrl::className());
            if (!plCtrl) {
                break;
            }

            plCtrl->offsetAllSelectedItems(nId == ID_PL_DOWN);
        }
        break;
    case ID_PL_PROPERTY:
        {
            MediaPtr media = g_player.getCurrentMedia();;
            CMPlaylistCtrl *plCtrl = (CMPlaylistCtrl*)m_pSkinWnd->getUIObjectByClassName(CMPlaylistCtrl::className());
            if (plCtrl && plCtrl->isVisible() && plCtrl->isParentVisible()) {
                int nSel;
                nSel = plCtrl->getNextSelectedItem(-1);
                if (nSel != -1) {
                    auto playlist = g_player.getNowPlaying();
                    media = playlist->getItem(nSel);
                }
            }

            if (media) {
                showMediaInfoDialog(m_pSkinWnd, media);
            }
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
