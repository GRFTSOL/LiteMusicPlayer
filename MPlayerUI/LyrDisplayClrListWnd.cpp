#include "MPlayerApp.h"
#include "LyrDisplayClrListWnd.h"


void createGradientFillImage(CRawImage &image, int nHeight, CColor clrGradient[3]);

string getPatternDir() {
    string strPatternDir = getAppResourceDir();
    strPatternDir += "Patterns" PATH_SEP_STR;

#if defined (_DEBUG) && defined (_WIN32)
    if (!isDirExist(strPatternDir.c_str())) {
        strPatternDir = getInstallShareDir();
        strPatternDir += "Patterns\\";
    }
#endif

    return strPatternDir;
}

CLyrDisplayClrListWnd::CLyrDisplayClrListWnd() {
    m_pWnd = nullptr;
    m_pNotify = nullptr;
    m_bInitialized = false;
    m_bHilight = true;
    m_et = ET_LYRICS_DISPLAY_SETTINGS;
    m_nWidthMax = 200;
}

void CLyrDisplayClrListWnd::create(CSkinWnd *pWnd, CRect &rc, bool bHighLight, bool bFloatingLyr, IPopupSkinWndNotify *pNotify) {
    m_pWnd = pWnd;
    m_pNotify = pNotify;

    if (!m_bInitialized) {
        m_popupListWnd.addImageColumn("", 100);
        m_bInitialized = true;
    }

    CMPlayerApp::getInstance()->getCurLyrDisplaySettingName(bFloatingLyr, m_strSectName, m_et);

    m_bHilight = bHighLight;

    m_nWidthMax = 0;
    m_popupListWnd.deleteAllItems();
    m_vFontClr.clear();

    CColor clr(RGB(255, 255, 255));
    profileGetColorValue(clr,
        m_strSectName.c_str(),
        getLyrTOBSettingName(m_bHilight, CLyricShowObj::TCI_FILL));
    addPureColor(clr.get());

    addGradientColors();

    addPatternImages();

    selectCurrentSetting();

    m_popupListWnd.setLineHeight(HEIGHT_COLOR);

    if (m_nWidthMax > rc.width()) {
        rc.right = rc.left + m_nWidthMax + 60;
    }

    // popup list window
    m_popupListWnd.create(pWnd->getSkinFactory(), "ComboBoxPopup.xml",
        pWnd, this, rc);
}

void CLyrDisplayClrListWnd::popupSkinWndOnSelected() {
    int nSel = m_popupListWnd.getCurSel();
    m_popupListWnd.postDestroy();

    if (nSel == -1) {
        return;
    }
    assert(nSel >= 0 && nSel < (int)m_vFontClr.size());

    // save selected color
    FontClrOpt &fontClr = m_vFontClr[nSel];
    string strColor;

    if (fontClr.obm == OBM_COLOR) {
        CDlgChooseColor dlg;

        if (dlg.doModal(m_pWnd, fontClr.clr[0]) != IDOK) {
            return;
        }

        fontClr.clr[0] = dlg.getColor().get();

        strColor = colorToStr(fontClr.clr[0]);
        CMPlayerSettings::setSettings(m_et, m_strSectName.c_str(),
            getLyrTOBSettingName(m_bHilight, CLyricShowObj::TCI_FILL), strColor.c_str());
    } else if (fontClr.obm == OBM_GRADIENT_COLOR) {
        for (int i = CLyricShowObj::TCI_GRADIENT1; i <= CLyricShowObj::TCI_GRADIENT3; i++) {
            strColor = colorToStr(fontClr.clr[i - CLyricShowObj::TCI_GRADIENT1]);
            CMPlayerSettings::setSettings(m_et, m_strSectName.c_str(),
                getLyrTOBSettingName(m_bHilight, (CLyricShowObj::TOBColorIndex)i), strColor.c_str());
        }
    } else {
        assert(fontClr.obm == OBM_PATTERN);
        CMPlayerSettings::setSettings(m_et, m_strSectName.c_str(),
            getLyrTOBSettingName(m_bHilight, CLyricShowObj::TCI_PATTERN), fontClr.strPatternFile.c_str());
    }

    CMPlayerSettings::setSettings(m_et, m_strSectName.c_str(),
        getLyrTOBSettingName(m_bHilight, CLyricShowObj::TCI_OBM), fontClr.obm);

    if (m_pNotify) {
        m_pNotify->popupSkinWndOnSelected();
    }
}

void CLyrDisplayClrListWnd::selectCurrentSetting() {
    CLyricShowObj::TextOverlayBlending tob;
    int nToSelect = 0;

    loadLyrOverlayBlendingSettings(m_strSectName.c_str(), tob, HEIGHT_COLOR, m_bHilight);

    if (tob.obm == OBM_COLOR) {
        nToSelect = 0;
        // addPureColor(tob.clr[CLyricShowObj::TCI_FILL].get());
    } else if (tob.obm == OBM_GRADIENT_COLOR) {
        nToSelect = findGradientColor(tob.clr[CLyricShowObj::TCI_GRADIENT1].get(),
            tob.clr[CLyricShowObj::TCI_GRADIENT2].get(),
            tob.clr[CLyricShowObj::TCI_GRADIENT3].get());
        if (nToSelect == -1) {
            addGradientColor(tob.clr[CLyricShowObj::TCI_GRADIENT1].get(),
                tob.clr[CLyricShowObj::TCI_GRADIENT2].get(),
                tob.clr[CLyricShowObj::TCI_GRADIENT3].get());
            nToSelect = (int)m_vFontClr.size() - 1;
        }
    } else if (tob.obm == OBM_PATTERN) {
        nToSelect = findPattern(tob.strPatternFile.c_str());
        if (nToSelect == -1 && tob.imgPattern.isValid()) {
            addImage(tob.imgPattern.getHandle());
            m_vFontClr.push_back(FontClrOpt(tob.strPatternFile.c_str()));
            tob.imgPattern.detach();
            nToSelect = (int)m_vFontClr.size() - 1;
        }
    }

    m_popupListWnd.setCurSel(nToSelect);
}

int CLyrDisplayClrListWnd::findGradientColor(COLORREF clr0, COLORREF clr1, COLORREF clr2) {
    for (int i = 0; i < (int)m_vFontClr.size(); i++) {
        FontClrOpt &opt = m_vFontClr[i];
        if (opt.obm == OBM_GRADIENT_COLOR) {
            if (opt.clr[0] == clr0 && opt.clr[1] == clr1 && opt.clr[2] == clr2) {
                return i;
            }
        }
    }

    return -1;
}

int CLyrDisplayClrListWnd::findPattern(cstr_t szPatternFile) {
    for (int i = 0; i < (int)m_vFontClr.size(); i++) {
        FontClrOpt &opt = m_vFontClr[i];
        if (opt.obm == OBM_PATTERN) {
            if (strcasecmp(opt.strPatternFile.c_str(), szPatternFile) == 0) {
                return i;
            }
        }
    }

    return -1;
}

void CLyrDisplayClrListWnd::addPureColor(COLORREF clr) {
    CColor color(clr);

    RawImageDataPtr image = createRawImageData(WIDTH_COLOR, HEIGHT_COLOR, 24);
    rawImageSet(image.get(), color.r(),
        color.g(),
        color.b(),
        255);

    addImage(image);
    m_vFontClr.push_back(FontClrOpt(clr));
}

void CLyrDisplayClrListWnd::addGradientColors() {
    addGradientColor(RGB(255, 255, 128), RGB(255, 128, 0), RGB(255, 128, 128));
    addGradientColor(RGB(128, 255, 255), RGB(0, 128, 255), RGB(128, 255, 255));
    addGradientColor(RGB(164, 164, 164), RGB(255, 255, 255), RGB(164, 164, 164));
    addGradientColor(RGB(164, 164, 164), RGB(255, 148, 0), RGB(164, 164, 164));
    // addGradientColor(RGB(, , ), RGB(, , ), RGB(, , ));
}

void CLyrDisplayClrListWnd::addGradientColor(COLORREF clr0, COLORREF clr1, COLORREF clr2) {
    CRawImage image;
    CColor clr[3];

    clr[0].set(clr0);
    clr[1].set(clr1);
    clr[2].set(clr2);

    createGradientFillImage(image, HEIGHT_COLOR, clr);

    if (image.isValid()) {
        addImage(image.getHandle());
        image.detach();

        m_vFontClr.push_back(FontClrOpt(clr0, clr1, clr2));
    }
}

void CLyrDisplayClrListWnd::addPatternImages() {
    string strPatternDir = getPatternDir();

    VecStrings vFiles;
    enumFilesInDir(strPatternDir.c_str(), nullptr, vFiles, true);

    for (int i = 0; i < (int)vFiles.size(); i++) {
        RawImageDataPtr image = loadRawImageDataFromFile(vFiles[i].c_str());
        if (image) {
            addImage(image);
            m_vFontClr.push_back(FontClrOpt(fileGetName(vFiles[i].c_str())));
        }
    }
}

void CLyrDisplayClrListWnd::addImage(const RawImageDataPtr &image) {
    assert(image);
    if (!image) {
        return;
    }

    assert(m_vFontClr.size() == m_popupListWnd.getItemCount());

    int n = m_popupListWnd.insertItem(m_popupListWnd.getItemCount(), "", 0, 0);

    m_popupListWnd.setItemImage(n, 0, image);
    // const int LINE_SPACE = 4;
    // if (m_popupListWnd.getLineHeight() < image->height + LINE_SPACE)
    //    m_popupListWnd.setLineHeight(image->height + LINE_SPACE);
    if (m_nWidthMax < image->width) {
        m_nWidthMax = image->width;
    }
}
