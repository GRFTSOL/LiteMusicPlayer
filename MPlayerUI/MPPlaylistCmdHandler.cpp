#include "MPlayerAppBase.h"
#include "MPPlaylistCmdHandler.h"
#include "MPHelper.h"
#include "DlgMediaInfo.h"
#include "MPlaylistCtrl.h"


CMPPlaylistCmdHandler::CMPPlaylistCmdHandler() {

}

CMPPlaylistCmdHandler::~CMPPlaylistCmdHandler() {

}

// if the command id is processed, return true.
bool CMPPlaylistCmdHandler::onCommand(int nId) {
    switch (nId) {
    case IDC_OPEN_PL:
        {
            CFileOpenDlg dlg("open Playlist", g_Player.getCurrentPlaylistFile().c_str(), "M3u file\0*.m3u\0All Files\0*.*\0\0", 0);
            if (dlg.doModal(m_pSkinWnd) == IDOK) {
                g_Player.loadPlaylist(dlg.getOpenFile(), true);
            }
        }
        break;
    case IDC_CLEAR:
        g_Player.clearPlaylist();
        break;
    default:
        return false;
    }

    return true;
}

bool CMPPlaylistCmdHandler::onCustomCommand(int nID) {
    switch (nID) {
    case CMD_PL_SAVE:
        {
            CFileSaveDlg dlg("save Playlist", g_Player.getCurrentPlaylistFile().c_str(), "M3u file\0*.m3u\0All Files\0*.*\0\0", 0);
            if (dlg.doModal(m_pSkinWnd) == IDOK) {
                if (isEmptyString(fileGetExt(dlg.getSaveFile()))) {
                    string file = dlg.getSaveFile();
                    fileSetExt(file, dlg.getSelectedExt());
                    g_Player.saveCurrentPlaylistAs(file.c_str());
                } else {
                    g_Player.saveCurrentPlaylistAs(dlg.getSaveFile());
                }
            }
        }
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
            CMPAutoPtr<IPlaylist> playlist;
            CMPAutoPtr<IMedia> media;

            CMPlaylistCtrl *plCtrl = (CMPlaylistCtrl*)m_pSkinWnd->getUIObjectByClassName(CMPlaylistCtrl::className());
            if (plCtrl && plCtrl->isVisible() && plCtrl->isParentVisible()) {
                int nSel;
                nSel = plCtrl->getNextSelectedItem(-1);
                if (nSel == -1) {
                    break;
                }

                if (g_Player.getCurrentPlaylist(&playlist) != ERR_OK) {
                    break;
                }

                if (playlist->getItem(nSel, &media) != ERR_OK) {
                    break;
                }
            } else {
                if (g_Player.getCurrentMedia(&media) != ERR_OK) {
                    break;
                }
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
