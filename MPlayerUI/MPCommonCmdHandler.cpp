#include "MPlayerApp.h"
#include "MLCmd.h"
#include "MPCommonCmdHandler.h"
#include "LyricShowAgentObj.h"
#include "LyricShowTextEditObj.h"
#include "SkinRateCtrl.h"
#include "DlgSearchLyrics.h"
#include "DlgUpload.h"
#include "AutoProcessEmbeddedLyrics.h"
#include "DlgSaveEmbeddedLyrics.h"
#include "PreferenceDlg.h"
#include "DlgAbout.h"
#include "MPSkinMenu.h"
#include "DownloadMgr.h"
#ifdef _WIN32
#include "win32/LyricsDownloader.h"
#include "DlgAdjustHue.h"
#endif

#include "MPHelper.h"
#include "MPFloatingLyrWnd.h"


bool g_bInModalDlLrcSelDlg;

const int SET_SPEED_SPAN = 500;

CMPCommonCmdHandler::CMPCommonCmdHandler(bool bFloatingLyr) {
    CMPlayerApp::getInstance()->getCurLyrDisplaySettingName(bFloatingLyr, m_strSectName, m_etDispSettings);
}

CMPCommonCmdHandler::~CMPCommonCmdHandler() {

}

// if the command id is processed, return true.
bool CMPCommonCmdHandler::onCommand(int nId) {
    switch (nId) {
    case IDC_SL_CLEAR_TIME_STAMP:
        {
            if (g_LyricData.getLyrContentType() == LCT_TXT) {
                CLyricShowTxtObj *pObj = (CLyricShowTxtObj*)m_pSkinWnd->getUIObjectByClassName(CLyricShowTxtObj::className());
                if (pObj) {
                    g_LyricData.clearRecordedLyrScrollActions(pObj->getLyrics());
                }
            }
        }
        break;
    case IDC_EDITOR_LYR_COLOR:
    case IDC_EDITOR_HIGH_COLOR:
    case IDC_TAG_COLOR:
    case IDC_EDIT_LINE_COLOR:
    case IDC_EDITOR_BG_COLOR:
        {
            cstr_t szColorValueName = nullptr;
            if (nId == IDC_EDITOR_LYR_COLOR) {
                szColorValueName = "Ed_LowColor";
            } else if (nId == IDC_EDITOR_HIGH_COLOR) {
                szColorValueName = "Ed_HighColor";
            } else if (nId == IDC_TAG_COLOR) {
                szColorValueName = "Ed_TagColor";
            } else if (nId == IDC_EDITOR_BG_COLOR) {
                szColorValueName = "Ed_BgColor";
            } else {
                szColorValueName = "Ed_FocusLineBgColor";
            }

            CColor clr(RGB(0, 0, 0));
            profileGetColorValue(clr, SZ_SECT_LYR_DISPLAY, szColorValueName);

            CDlgChooseColor dlg;
            if (dlg.doModal(m_pSkinWnd, clr) == IDOK) {
                clr = dlg.getColor();
                CMPlayerSettings::setSettings(ET_LYRICS_DISPLAY_SETTINGS,
                    SZ_SECT_LYR_DISPLAY, szColorValueName, colorToStr(clr).c_str());
            }
        }
        break;
    default:
        {
            if (onCommandCharEncoding(nId)) {
                break;
            }
            if (onCommandSkin(nId)) {
                break;
            }
            return false;
        }
    }

    return true;
}

bool CMPCommonCmdHandler::onCustomCommand(int nID) {
    switch (nID) {
    case CMD_NO_SUITTABLE_LYRICS:
    case CMD_INSTRUMENTAL_MUSIC:
        if (g_LyricSearch.associateLyrics(g_Player.getMediaKey().c_str(), NONE_LYRCS)) {
            CMPlayerAppBase::getInstance()->dispatchResearchLyrics();
        }
        break;
    case CMD_SEARCH_LYR_SUGGESTIONS:
        openUrl(m_pSkinWnd, getStrName(SN_HTTP_HELP_SEARCH_LYR_SUGGESTIONS));
        break;

    case CMD_RATE_LYR_1:
    case CMD_RATE_LYR_2:
    case CMD_RATE_LYR_3:
    case CMD_RATE_LYR_4:
    case CMD_RATE_LYR_5:
        {
            int nRate;
            if (nID == CMD_RATE_LYR_2) nRate = 2;
            else if (nID == CMD_RATE_LYR_3) nRate = 3;
            else if (nID == CMD_RATE_LYR_4) nRate = 4;
            else if (nID == CMD_RATE_LYR_5) nRate = 5;
            else nRate = 1;

            string strUrl;

            strUrl = getStrName(SN_HTTP_RATE_LRC);
            strUrl += g_LyricData.properties().m_strId;

            strUrl += stringPrintf("&rating=%d", nRate).c_str();
            openUrl(m_pSkinWnd, strUrl.c_str());
        }
        break;
    case CMD_LDO_KARAOKE:
        CMPlayerSettings::setSettings(m_etDispSettings, m_strSectName.c_str(), "Karaoke",
            !g_profile.getBool(m_strSectName.c_str(), "Karaoke", false));
        break;
    case CMD_LDO_NORMAL:
    case CMD_LDO_FADE_IN:
    case CMD_LDO_FADEOUT_BG:
    case CMD_LDO_AUTO:
        {
            DISPLAY_OPTIONS displayOpt;

            if (nID == CMD_LDO_NORMAL) {
                displayOpt = DO_NORMAL;
            } else if (nID == CMD_LDO_FADE_IN) {
                displayOpt = DO_FADEOUT_LOWCOLOR;
            } else if (nID == CMD_LDO_FADEOUT_BG) {
                displayOpt = DO_FADEOUT_BG;
            } else if (nID == CMD_LDO_AUTO) {
                displayOpt = DO_AUTO;
            } else {
                return false;
            }

            CMPlayerSettings::setSettings(m_etDispSettings, m_strSectName.c_str(), "LyrDrawOpt",
                displayOptToStr(displayOpt));
        }
        break;

#ifdef _WIN32_DESKTOP
    case CMD_TOGGLE_MP:
        {
            if (m_pSkinWnd->isIconic() || ::GetForegroundWindow() != m_pSkinWnd->getHandle()) {
                m_pSkinWnd->activateWindow();
            } else {
                m_pSkinWnd->showWindow(SW_MINIMIZE);
                if (m_pSkinWnd->isToolWindow()) {
                    m_pSkinWnd->showWindow(SW_HIDE);
                }
            }
        }
        break;
    case CMD_ADJUST_HUE:
        {
            showAdjustHueDialog(m_pSkinWnd);
        }
        break;
#endif // _WIN32_DESKTOP

    case CMD_FLOATING_LYRICS:
        {
            g_profile.writeInt("FloatingLyr", !g_wndFloatingLyr.isValid());
            if (!g_wndFloatingLyr.isValid()) {
                g_wndFloatingLyr.create();
            } else {
                g_wndFloatingLyr.destroy();
            }
        }
        break;

    case CMD_PREFERENCES:
        showPreferenceDialog(m_pSkinWnd, m_etDispSettings != ET_LYRICS_FLOATING_SETTINGS);
        break;
    case CMD_DISPLAY_OPT:
        showPreferenceDialog(m_pSkinWnd, m_etDispSettings != ET_LYRICS_FLOATING_SETTINGS, PAGE_LYR_DISPLAY);
        break;

    case CMD_WEBHOME:
        openUrl(m_pSkinWnd, getStrName(SN_HTTP_DOMAIN));
        break;
    case CMD_EMAIL:
        // feed back
        {
            string str;

            str = getStrName(SN_EMAIL);
            str += "?subject=feedback";

            openUrl(m_pSkinWnd, str.c_str());
        }
        break;
        //     case CMD_MINIMIZE:
        //         CMPlayerAppBase::getMPSkinFactory()->minizeAll();
        //         break;
    case CMD_APPLY_ACCOUNT:
        {
            openUrl(m_pSkinWnd, getStrName(SN_HTTP_SIGNUP));
        }
        break;
    case CMD_LOGIN_VIA_IE:
        {
            openUrl(m_pSkinWnd, getStrName(SN_HTTP_LOGIN));
        }
        break;
    case CMD_ABOUT:
        {
            /*            CSkinWnd    *pWnd = new CDlgAbout();
            int nRet = CMPlayerAppBase::getMPSkinFactory()->openOrCloseSkinWnd("about_box",
                "AboutBox",
                "AboutBox.xml", CMPlayerAppBase::getMainWnd(), &pWnd);*/
            showAboutDialog(m_pSkinWnd);
        }
        break;
    case CMD_BR_ALBUM_ART:
        {
            string strAlbumArtFileName = fileGetPath(g_Player.getSrcMedia());
            if (!isDirExist(strAlbumArtFileName.c_str())) {
                break;
            }

            if (isEmptyString(g_Player.getAlbum())) {
                strAlbumArtFileName += "Folder";
            } else {
                strAlbumArtFileName += g_Player.getAlbum();
            }

            CFileDlgExtFilter strFileExt;
            strFileExt.addExtention(_TLT("All Picture Files"), "*.jpg;*.gif;*.bmp;*.png");
            strFileExt.addExtention("JPEG (*.jpg)", "*.jpg");
            strFileExt.addExtention("PNG (*.png)", "*.png");
            strFileExt.addExtention(_TLT("Bitmap files (*.bmp)"), "*.bmp");
            CFileOpenDlg        dlg(_TLT("Browse album art picture file"), "",
                strFileExt.c_str(), 0);

            if (dlg.doModal(m_pSkinWnd) != IDOK) {
                break;
            }

            strAlbumArtFileName += fileGetExt(dlg.getOpenFile());

            if (strcasecmp(strAlbumArtFileName.c_str(), dlg.getOpenFile()) == 0) {
                break;
            }

            copyFile(dlg.getOpenFile(), strAlbumArtFileName.c_str(), false);

            CMPlayerAppBase::getEventsDispatcher()->dispatchSyncEvent(ET_PLAYER_CUR_MEDIA_INFO_CHANGED);
        }
        break;
    case CMD_HELP_STATIC_LYR:
        openUrl(m_pSkinWnd, getStrName(SN_HTTP_HELP_STATIC_LYR));
        break;

    case CMD_LYR_SCROLL_ENABLE_RECORD:
    case CMD_LYR_SCROLL_ENABLE_REPLAY:
        {
            CLyricShowTxtObj *pObj = (CLyricShowTxtObj*)m_pSkinWnd->getUIObjectByClassName(CLyricShowTxtObj::className());
            if (pObj) {
                if (nID == CMD_LYR_SCROLL_ENABLE_RECORD) {
                    pObj->enableRecordScrollingActions(!pObj->isRecordScrollingActionsEnabled());
                } else {
                    pObj->enableReplayScrollingActions(!pObj->isReplayScrollingActionsEnabled());
                }
            }
        }
        break;

    case CMD_LYR_SCROLL_MENU:
        {
            CMenu *menu = nullptr;
            CPoint pt = getCursorPos();
            if (m_pSkinWnd->getSkinFactory()->loadMenu(m_pSkinWnd, &menu, "StaticLyricsMenu")) {
                menu->enableItem(IDC_SL_CLEAR_TIME_STAMP, g_LyricData.getLyrContentType() == LCT_TXT);
                menu->trackPopupMenu(pt.x, pt.y, m_pSkinWnd, nullptr);
                delete menu;
            }
        }
        break;
    case CMD_LYR_EDITOR:
        {
            SkinWndStartupInfo skinWndStartupInfo("LyrEditor", _TLT("Lyrics Editor"), "LyricsEditor.xml", nullptr);
            CMPlayerAppBase::getMPSkinFactory()->activeOrCreateSkinWnd(skinWndStartupInfo);
        }
        break;
    case CMD_EDIT_HELP:
        openUrl(m_pSkinWnd, getStrName(SN_HTTP_HELP_EDIT_LYR));
        break;
    case CMD_HELP:
        {
            CUIObject *pObjLyrEdit = m_pSkinWnd->getUIObjectByClassName(CLyricShowTextEditObj::className());
            if (pObjLyrEdit && pObjLyrEdit->isVisible()) {
                openUrl(m_pSkinWnd, getStrName(SN_HTTP_HELP_EDIT_LYR));
            } else {
                openUrl(m_pSkinWnd, getStrName(SN_HTTP_HELP));
            }
        }
        break;
    case CMD_PAUSE:
        g_Player.pause();
        break;
    case CMD_PLAY:
        g_Player.play();
        break;
    case CMD_PLAYPAUSE:
        g_Player.playPause();
        break;
    case CMD_STOP:
        g_Player.stop();
        break;
    case CMD_NEXT:
        g_Player.next();
        break;
    case CMD_PREVIOUS:
        g_Player.prev();
        break;
    case CMD_SEEK:
        {
            CSkinSeekCtrl *pCtrl = (CSkinSeekCtrl*)m_pSkinWnd->getUIObjectById(CMD_SEEK, CSkinSeekCtrl::className());
            if (pCtrl) {
                g_Player.seekTo(pCtrl->getScrollPos());
            }
        }
        break;
    case CMD_BACKWARD:
        {
            int dwCurPos;

            dwCurPos = g_Player.getPlayPos();
            dwCurPos -= 2000;
            if (dwCurPos < 0) {
                dwCurPos = 0;
            }
            dwCurPos -= dwCurPos % 1000;
            g_Player.seekTo(dwCurPos);
        }
        break;
    case CMD_FORWARD:
        {
            int dwCurPos;

            dwCurPos = g_Player.getPlayPos();
            dwCurPos += 2000;
            if (dwCurPos < 0) {
                dwCurPos = 0;
            }
            dwCurPos -= dwCurPos % 1000;
            g_Player.seekTo(dwCurPos);
        }
        break;
    case CMD_VOL_INC:
        {
            int vol = g_Player.getVolume();
            if (vol < MP_VOLUME_MAX) {
                vol += 5;
                vol -= vol % 5;
                if (vol > MP_VOLUME_MAX) {
                    vol = MP_VOLUME_MAX;
                }
                g_Player.setVolume(vol);
                CMPlayerAppBase::getInstance()->dispatchInfoText(stringPrintf("%s %d%%", _TLT("set Volume"), vol).c_str());
            }
        }
        break;
    case CMD_VOL_DEC:
        {
            int vol = g_Player.getVolume();
            if (vol > 0) {
                vol -= 5;
                vol -= vol % 5;
                if (vol < 0) {
                    vol = 0;
                }
                g_Player.setVolume(vol);
                CMPlayerAppBase::getInstance()->dispatchInfoText(stringPrintf("%s %d%%", _TLT("set Volume"), vol).c_str());
            }
        }
        break;
    case CMD_MUTE:
        {
            bool bMute = !g_Player.isMute();
            g_Player.setMute(bMute);
            CMPlayerAppBase::getInstance()->dispatchInfoText(_TL(bMute ? "Mute On" : "Mute Off"));
        }
        break;
    case CMD_SHUFFLE:
        {
            bool bShuffle = !g_Player.isShuffle();
            g_Player.setShuffle(bShuffle);
            CMPlayerAppBase::getInstance()->dispatchInfoText(_TL(bShuffle ? "Shuffle On" : "Shuffle Off"));
        }
        break;
    case CMD_LOOP_OFF:
        g_Player.setLoop(MP_LOOP_OFF);
        break;
    case CMD_LOOP_ALL:
        g_Player.setLoop(MP_LOOP_ALL);
        break;
    case CMD_LOOP_TRACK:
        g_Player.setLoop(MP_LOOP_TRACK);
        break;
    case CMD_LOOP:
        {
            g_Player.setToNextLoopMode();

            string str;
            MP_LOOP_MODE loopMode = g_Player.getLoop();
            if (loopMode == MP_LOOP_OFF) {
                str = "Repeat Off";
            } else if (loopMode == MP_LOOP_TRACK) {
                str =  "Repeat Track";
            } else {
                str =  "Repeat All";
            }

            CMPlayerAppBase::getInstance()->dispatchInfoText(str.c_str());
        }
        break;
    case CMD_RATE:
        {
            CSkinRateCtrl *pRateCtrl;
            pRateCtrl = (CSkinRateCtrl*)m_pSkinWnd->getUIObjectById(CMD_RATE, CSkinRateCtrl::className());
            if (pRateCtrl) {
                CMPAutoPtr<IMediaLibrary> pMediaLib;
                CMPAutoPtr<IMedia> pMedia;

                if (g_Player.getMediaLibrary(&pMediaLib) != ERR_OK) {
                    break;
                }

                if (g_Player.getCurrentMedia(&pMedia) == ERR_OK) {
                    pMediaLib->rate(pMedia, pRateCtrl->getRating());
                }
            }
        }
        break;
    case CMD_PL_OPEN_FILE:
        onSongOpenFileCmd(m_pSkinWnd, true);
        break;
    case CMD_PL_ADD_FILE:
        onSongOpenFileCmd(m_pSkinWnd, false);
        break;
    case CMD_PL_OPEN_DIR:
        onSongOpenDirCmd(m_pSkinWnd, true);
        break;
    case CMD_PL_ADD_DIR:
        onSongOpenDirCmd(m_pSkinWnd, false);
        break;

        //
        // Lyrics ....
        //
    case CMD_NEW_LRC:
        {
            CMPlayerAppBase::getInstance()->getEventsDispatcher()->dispatchSyncEvent(ET_LYRICS_ON_SAVE_EDIT);

            if (g_LyricData.isContentModified()) {
                string        strMessage = stringPrintf(_TLT("The lyrics of the %s file have changed."),
                    fileGetName(g_LyricData.getSongFileName())).c_str();
                strMessage += "\r\n\r\n";
                strMessage += _TLT("Do you want to save the changes?");
                int nRet = m_pSkinWnd->messageOut(strMessage.c_str(), MB_ICONQUESTION | MB_YESNOCANCEL);
                if (nRet == IDCANCEL) {
                    break;
                } else if (nRet == IDYES) {
                    if (!CMPCommonCmdHandler::saveCurrentLyrics(m_pSkinWnd, false)) {
                        break;
                    }
                }
            }
            CMPlayerAppBase::getInstance()->newLyrics();
        }
        break;
    case CMD_OPEN_LRC:
        //
        // 歌词下载
        //
        if (!g_Player.isMediaOpened()) {
            m_pSkinWnd->messageOut(_TLT("No media was opened."));
            break;
        }

        {
            if (!g_bInModalDlLrcSelDlg) {
                g_bInModalDlLrcSelDlg = true;

                showSearchLyricsDialog(m_pSkinWnd);
                g_bInModalDlLrcSelDlg = false;
            }
        }
        break;
    case CMD_SAVE_LRC:
        saveCurrentLyrics(m_pSkinWnd, true);
        break;
    case CMD_SAVE_LRC_AS:
        {
            // notify to save...
            IEvent *pEvent = new IEvent;
            pEvent->eventType = ET_LYRICS_ON_SAVE_EDIT;
            CMPlayerAppBase::getEventsDispatcher()->dispatchSyncEvent(pEvent);

            saveAsLyricsFile(m_pSkinWnd,
                lyricsConentTypeToFileType(g_LyricData.getLyrContentType()));
        }
        break;
    case CMD_SAVE_LYR_IN_SONG_FILE:
        {
            CDlgSaveEmbeddedLyrics dlg;
            dlg.doModal(m_pSkinWnd);
        }
        break;
    case CMD_FORWARD_LYRICS:
        {
            CUIObject *pObj;
            pObj = m_pSkinWnd->getUIObjectByClassName(CLyricShowTxtObj::className());
            if (pObj) {
                if (pObj) {
                    pObj->onKeyDown(VK_DOWN, 0);
                }
            } else {
                g_LyricData.setOffsetTime(g_LyricData.getOffsetTime() + SET_SPEED_SPAN);
                CMPlayerAppBase::getEventsDispatcher()->dispatchSyncEvent(ET_LYRICS_DRAW_UPDATE);

                CMPlayerAppBase::getEventsDispatcher()->dispatchSyncEvent(ET_LYRICS_EDITOR_RELOAD_TAG);

                CMPlayerAppBase::getInstance()->dispatchInfoText(_TLT("Lyrics Forward 0.5 Sec"));
            }
        }
        break;
    case CMD_BACKWARD_LYRICS:
        {
            CUIObject *pObj;
            pObj = m_pSkinWnd->getUIObjectByClassName(CLyricShowTxtObj::className());
            if (pObj) {
                // move down one line.
                if (pObj) {
                    pObj->onKeyDown(VK_UP, 0);
                }
            } else {
                g_LyricData.setOffsetTime(g_LyricData.getOffsetTime() - SET_SPEED_SPAN);
                CMPlayerAppBase::getEventsDispatcher()->dispatchSyncEvent(ET_LYRICS_DRAW_UPDATE);

                CMPlayerAppBase::getEventsDispatcher()->dispatchSyncEvent(ET_LYRICS_EDITOR_RELOAD_TAG);

                CMPlayerAppBase::getInstance()->dispatchInfoText(_TLT("Lyrics Backward 0.5 Sec"));
            }
        }
        break;

    case CMD_FONT_SIZE_INC:
    case CMD_FONT_SIZE_DEC:
        {
            // increase the font size.
            FontInfoEx font;

            // get default settings in profile
            profileGetLyricsFont(m_strSectName.c_str(), font);

            if (nID == CMD_FONT_SIZE_INC) {
                font.height++;
                if (font.height >= 96 * 2) { // 96 is the max font size in font choosing dialog.
                    break;
                }
            } else {
                font.height--;
                if (font.height < 8) { // 8 is a reasonable smallest font size.
                    break;
                }
            }

            // now save to profile
            profileWriteLyricsFont(m_etDispSettings, m_strSectName.c_str(), font);
        }
        break;
    case CMD_CLR_PREV_HUE:
    case CMD_CLR_NEXT_HUE:
        {
            float hue = (float)(g_profile.getInt(CMPlayerAppBase::getMPSkinFactory()->getSkinName(), "Hue", 0));

            if (nID == CMD_CLR_NEXT_HUE) {
                hue += 30;
            } else {
                hue -= 30;
            }
            if (hue >= 360)   hue -= 360;
            else if (hue < 0) hue += 360;

            g_profile.writeInt(CMPlayerAppBase::getMPSkinFactory()->getSkinName(), "Hue", (int)hue);
            CMPlayerAppBase::getMPSkinFactory()->adjustHue(hue);
        }
        break;
    case CMD_LYR_HIGH_CLR_LIST:
        {
            CRect rc;
            CUIObject *pObj = m_pSkinWnd->getUIObjectById(CMD_LYR_HIGH_CLR_LIST, nullptr);
            if (pObj) {
                rc = pObj->m_rcObj;
                m_pSkinWnd->screenToClient(rc);
            } else {
                CPoint pt = getCursorPos();
                rc.setLTRB(pt.x, pt.y, pt.x + 100, pt.y + 25);
            }
            m_popupHighClrListWnd.create(m_pSkinWnd, rc, true,
                m_etDispSettings == ET_LYRICS_FLOATING_SETTINGS, nullptr);
        }
        break;

    case CMD_OK:
    case CMD_CANCEL:
    case ID_ALBUMART:
        {
            // album art
            m_pSkinWnd->getSkinFactory()->showPopupMenu(m_pSkinWnd, "AlbumArtMenu");
        }
        break;
    case CMD_RATE_LYR:
        {
            if (!g_LyricData.hasLyricsOpened()) {
                m_pSkinWnd->messageOut(_TLT("No Lyrics file was opened."));
                break;
            }

            m_pSkinWnd->getSkinFactory()->showPopupMenu(m_pSkinWnd, "RateMenu");
        }
        break;
    case CMD_RELOAD_LYR:
        CMPlayerAppBase::getInstance()->dispatchResearchLyrics();
        break;
    case CMD_J_PREV_LINE:
    case CMD_J_NEXT_LINE:
        {
            // Jump to next/previous line of lyrics.
            CLyricsLines &vLyrics = g_LyricData.getRawLyrics();

            int nCurLine = g_LyricData.getCurPlayLine(vLyrics);
            if (nCurLine >= 0 && nCurLine < (int)vLyrics.size()) {
                if (nID == CMD_J_NEXT_LINE) {
                    if (nCurLine == vLyrics.size() - 1) {
                        return true;
                    }
                    nCurLine++;
                } else {
                    if (nCurLine > 0) {
                        nCurLine--;
                    }
                }

                g_Player.seekTo(vLyrics[nCurLine]->nBegTime);
            }
        }
        break;
    case CMD_UPLOAD_LYR:
        {
            if (!g_LyricData.hasLyricsOpened()) {
                m_pSkinWnd->messageOut(_TLT("No Lyrics file was opened."));
                break;
            }

            // show upload lyrics dialog
            string strLyrics;
            g_LyricData.toString(strLyrics, FT_LYRICS_LRC, true);
            showUploadLyrDialog(m_pSkinWnd,
                strLyrics,
                g_LyricData.getSongFileName(),
                g_LyricData.getLyricsFileName());
        }
        break;
    case CMD_EXTERNAL_LYR_EDIT:
        //
        // Edit lyrics with external editor.
        if (g_LyricData.getLyricsSourceType() != LST_FILE) {
            m_pSkinWnd->messageOut(_TLT("Embedded lyrics can not be edited with external editors."));
            break;
        }

        if (g_LyricData.hasLyricsOpened()) {
            if (!CMPlayerAppBase::getInstance()->onLyricsChangingSavePrompt()) {
                break;
            }

            string strEditor;
            getNotepadEditor(strEditor);
            execute(m_pSkinWnd,
                CMLProfile::getDir(SZ_SECT_UI, "LyricsEditor", strEditor.c_str()).c_str(),
                g_LyricData.getLyricsFileName());
        }
        break;
    default:
        return false;
    }

    return true;
}

bool CMPCommonCmdHandler::onUIObjNotify(IUIObjNotify *pNotify) {
    if (pNotify->nID == CMD_SEEK) {
        if (pNotify->pUIObject->isKindOf(CSkinSeekCtrl::className())) {
            CSkinSeekCtrlEventNotify *pListCtrlNotify = (CSkinSeekCtrlEventNotify *)pNotify;
            if (pListCtrlNotify->cmd == CSkinSeekCtrlEventNotify::C_BEG_DRAG) {
                // Use seek time instead of playing time.
                g_Player.setUseSeekTimeAsPlayingTime(true);
            } else if (pListCtrlNotify->cmd == CSkinSeekCtrlEventNotify::C_END_DRAG) {
                // Use seek time instead of playing time.
                g_Player.setUseSeekTimeAsPlayingTime(false);
            }
        }
    } else {
        return false;
    }

    return true;
}

bool CMPCommonCmdHandler::getChecked(uint32_t nID, bool &bChecked) {
    switch (nID) {
    case IDC_LDO_KARAOKE:
        bChecked = g_profile.getBool(m_strSectName.c_str(), "Karaoke", false);
        break;
    case IDC_FLOATING_LYRICS:
        bChecked = g_wndFloatingLyr.isValid();
        break;
    default:
        return false;
    }

    return true;
}

bool CMPCommonCmdHandler::getRadioChecked(vector<uint32_t> &vIDs, uint32_t &nIDChecked) {
    switch (vIDs[0]) {
    case IDC_LDO_NORMAL:
    case IDC_LDO_FADE_IN:
    case IDC_LDO_FADEOUT_BG:
    case IDC_LDO_AUTO:
        {
            DISPLAY_OPTIONS        displayOpt = displayOptFromStr(
                g_profile.getString(m_strSectName.c_str(), "LyrDrawOpt", ""));

            if (displayOpt == DO_NORMAL) {
                nIDChecked = IDC_LDO_NORMAL;
            } else if (displayOpt == DO_FADEOUT_LOWCOLOR) {
                nIDChecked = IDC_LDO_FADE_IN;
            } else if (displayOpt == DO_FADEOUT_BG) {
                nIDChecked = IDC_LDO_FADEOUT_BG;
            } else {
                nIDChecked = IDC_LDO_AUTO;
            }
        }
        break;
        //     case IDC_ALIGN_LYRICS_LEFT:
        //     case IDC_ALIGN_LYRICS_CENTER:
        //     case IDC_ALIGN_LYRICS_RIGHT:
        //         {
        //             string        strAlign;
        //             strAlign = g_profile.getString(m_strSectName.c_str(), "LyrAlign", "center");
        //             if (strcasecmp(strAlign.c_str(), "left") == 0)
        //                 nIDChecked = IDC_ALIGN_LYRICS_LEFT;
        //             else if (strcasecmp(strAlign.c_str(), "right") == 0)
        //                 nIDChecked = IDC_ALIGN_LYRICS_RIGHT;
        //             else
        //                 nIDChecked = IDC_ALIGN_LYRICS_CENTER;
        //         }
        //         break;
    default:
        return false;
    }

    return true;
}

bool CMPCommonCmdHandler::onCommandCharEncoding(int nCmdId) {
    int nEncodingId = cmdIdToEncoding(nCmdId);
    if (nEncodingId == -1) {
        return false;
    }

    if (!g_LyricData.hasLyricsOpened()) {
        return true;
    }

    //
    // if (修改了歌词)
    //        提示需要重新加载歌词，提示用户是否继续
    if (g_LyricData.isContentModified()) {
        string str;

        str += _TLT("Change character encoding must reload lyrics, and your modification will be lost.");
        str += "\r\n";
        str += _TLT("Do you want to continue?");
        if (m_pSkinWnd->messageOut(str.c_str(), MB_ICONINFORMATION | MB_YESNO) != IDYES) {
            return true;
        }
    }

    // 以用户指定的编码重新打开歌词
    g_LyricData.reopenLyrics(true, (CharEncodingType)nEncodingId);

    CMPlayerAppBase::getInstance()->dispatchLyricsChangedSyncEvent();

    return true;
}

// COMMENT:
//        保存歌词文件为：
//        FT_UNKNOWN            = 1,
//        FT_LYRICS_LRC        = 2,
//        FT_LYRICS_TXT        = 3,
// RETURN:
//        true    -    保存成功，注意：如果没有歌词数据也返回true；
//        false    -    用户选择取消，或者保存失败;
bool CMPCommonCmdHandler::saveAsLyricsFile(Window *pWndParent, MLFileType DefFileType) {
    if (!g_LyricData.hasLyricsOpened()) {
        pWndParent->messageOut(_TLT("No Lyrics file was opened."));
        return true;
    }

    string strFile = g_LyricData.getLyricsFileName();
    if (strFile.empty() || !isFileExist(strFile.c_str())) {
        strFile = g_LyricsDownloader.getSaveLyricsFile(g_LyricData.getSongFileName(),
            g_LyricData.getSuggestedLyricsFileName().c_str());
    }

    // set saved file extension
    if (DefFileType == FT_LYRICS_LRC) {
        fileSetExt(strFile, ".lrc");
    } else if (DefFileType == FT_LYRICS_TXT) {
        fileSetExt(strFile, ".txt");
    } else if (DefFileType == FT_LYRICS_SNC) {
        fileSetExt(strFile, ".snc");
    }

    char szWndTitle[256];
    int nFileExtIndex;
    if (DefFileType == FT_LYRICS_SNC) {
        nFileExtIndex = 4;
    } else if (DefFileType == FT_LYRICS_TXT) {
        nFileExtIndex = 3;
    } else if (DefFileType == FT_LYRICS_LRC) {
        nFileExtIndex = 2;
    } else {
        nFileExtIndex = 1;
    }

    strcpy_safe(szWndTitle, CountOf(szWndTitle), _TLT("save As"));
    CFileSaveDlg        dlg(szWndTitle, strFile.c_str(),
        "All supported files (*.lrc; *.txt; *.snc)\0*.lrc;*.txt;*.snc\0LRC Lyrics File (*.lrc)\0*.lrc\0Text File (*.txt)\0*.txt\0Snc File (*.snc)\0*.snc\0\0",
        nFileExtIndex);

    if (dlg.doModal(pWndParent) == IDOK) {
        strFile = dlg.getSaveFile();
        if (isEmptyString(fileGetExt(strFile.c_str()))) {
            // set file extend name as the user selected.
            fileSetExt(strFile, dlg.getSelectedExt());
        }
        MLFileType fileType = GetLyricsFileType(strFile.c_str());
        if (fileType != FT_LYRICS_LRC && fileType != FT_LYRICS_TXT
            && fileType != FT_LYRICS_SNC) {
            return false;
        }

        bool bUseNewFileName;
        int nRet = g_LyricData.saveAsFile(strFile.c_str(), bUseNewFileName);
        if (nRet != ERR_OK) {
            pWndParent->messageOut(ERROR2STR_LOCAL(nRet));
            return false;
        }
        if (bUseNewFileName) {
            g_LyricSearch.associateLyrics(g_LyricData.getSongFileName(), g_LyricData.getLyricsFileName());
        }
    } else {
        return false;
    }

    return true;
}

bool CMPCommonCmdHandler::saveCurrentLyrics(CSkinWnd *pSkinWnd, bool bDispatchOnSave) {
    int nRet;

    if (bDispatchOnSave) {
        // Dispatch on save lyrics message.
        IEvent *pEvent = new IEvent;
        pEvent->eventType = ET_LYRICS_ON_SAVE_EDIT;
        CMPlayerAppBase::getEventsDispatcher()->dispatchSyncEvent(pEvent);
    }

    if (g_LyricData.doesChooseNewFileName()) {
        if (!saveAsLyricsFile(pSkinWnd,
            lyricsConentTypeToFileType(g_LyricData.getLyrContentType()))) {
            return false;
        }
    } else {
        nRet = g_LyricData.save();
        if (nRet != ERR_OK) {
            if (g_LyricData.getLyricsSourceType() != LST_FILE) {
                string str;
                g_LyricData.toString(str, FT_LYRICS_LRC, true);

                VecStrings vLyrNames;
                vLyrNames.push_back(g_LyricData.getLyricsFileName());

                string bufLyrics = insertWithFileBom(str);
                g_autoProcessEmbeddedLyrics.saveEmbeddedLyrics(g_LyricData.getSongFileName(),
                    nullptr, &bufLyrics, vLyrNames);
            }
            pSkinWnd->messageOut(stringPrintf("%s\n%s", ERROR2STR_LOCAL(nRet), _TLT("Failed to save embedded lyrics, $Product$ will auto try again later.")).c_str());
        }
    }

    return true;
}

CLyricsLines &CMPCommonCmdHandler::getDisplayLyrics() {
    CLyricShowObj *pObj = (CLyricShowObj*)m_pSkinWnd->getUIObjectByClassName(CLyricShowMultiRowObj::className());
    if (pObj) {
        return pObj->getLyrics();
    } else {
        return g_LyricData.getRawLyrics();
    }
}
