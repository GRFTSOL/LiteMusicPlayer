#ifndef SKIN_EDITOR
#include "MPlayerAppBase.h"
#endif
#include "MPSkinVis.h"


UIOBJECT_CLASS_NAME_IMP(CMPSkinVis, "Vis")

CMPSkinVis::CMPSkinVis() {
    m_msgNeed = UO_MSG_WANT_LBUTTON;
    m_clrFg.set(RGB(0, 255, 0));
    m_pen.createSolidPen(1, m_clrFg);

    m_nLeftMargin = m_nRightMargin = m_nTopMargin = m_nBottomMargin = 0;
    m_nSpectrumColSpace = 0;
    m_nSpectrumUnitHeight = 1;
    m_visParamCur = nullptr;
    if (strcasecmp(g_profile.getString("VisMode", ""),  "Osilloscope") == 0) {
        m_visMode = VM_OSILLOSCOPE;
    } else {
        m_visMode = VM_SPECTRUM;
    }
}

CMPSkinVis::~CMPSkinVis() {
#ifndef SKIN_EDITOR
    CMPlayerAppBase::getEventsDispatcher()->unRegisterHandler(ET_VIS_DRAW_UPDATE, this);
#endif
}

#ifndef SKIN_EDITOR
void CMPSkinVis::onCreate() {
    CUIObject::onCreate();

    CMPlayerAppBase::getEventsDispatcher()->registerHandler(ET_VIS_DRAW_UPDATE, this);
}

void CMPSkinVis::onEvent(const IEvent *pEvent) {
    CEventVisDrawUpdate *pVisEvt = (CEventVisDrawUpdate*)pEvent;

    //// the pVisEvt->pVisParam should be a static param.
    m_visParamCur = pVisEvt->pVisParam;

    invalidate();
}
#endif

bool CMPSkinVis::onLButtonUp(uint32_t nFlags, CPoint point) {

    if (m_visMode == VM_OSILLOSCOPE) {
        m_visMode = VM_SPECTRUM;
        g_profile.writeString("VisMode", "Spectrum");
    } else {
        m_visMode = VM_OSILLOSCOPE;
        g_profile.writeString("VisMode", "Osilloscope");
    }


    return true;
}

void CMPSkinVis::draw(CRawGraph *canvas) {
    if (m_visParamCur) {
        if (m_visMode == VM_SPECTRUM) {
            drawSpectrum(canvas, m_visParamCur);
        } else {
            drawOsilloscope(canvas, m_visParamCur);
        }
    } else {
        m_imgBg.blt(canvas, m_rcObj.left, m_rcObj.top, m_rcObj.width(), m_rcObj.height(), m_imgBg.m_x, m_imgBg.m_y);
    }
}

bool CMPSkinVis::setProperty(cstr_t szProperty, cstr_t szValue) {
    if (CUIObject::setProperty(szProperty, szValue)) {
        return true;
    }

    if (strcasecmp(szProperty, SZ_PN_IMAGE) == 0) {
        m_strBgFile = szValue;
        m_imgBg.loadFromSRM(m_pSkin, szValue);
    } else if (strcasecmp(szProperty, SZ_PN_IMAGERECT) == 0) {
        if (!getRectValue(szValue, m_imgBg)) {
            ERR_LOG2("Analyse Value: %s = %s FAILED.", szProperty, szValue);
        }
    }
    if (strcasecmp(szProperty, "SpectrumColImage") == 0) {
        m_strSpectrumColFile = szValue;
        m_imgSpectrumCol.loadFromSRM(m_pSkin, szValue);
    } else if (strcasecmp(szProperty, "SpectrumColRect") == 0) {
        if (!getRectValue(szValue, m_imgSpectrumCol)) {
            ERR_LOG2("Analyse Value: %s = %s FAILED.", szProperty, szValue);
        }
    } else if (strcasecmp(szProperty, "SpectrumColSpace") == 0) {
        m_nSpectrumColSpace = atoi(szValue);
    } else if (strcasecmp(szProperty, "SpectrumUnitHeight") == 0) {
        m_nSpectrumUnitHeight = atoi(szValue);
    } else if (strcasecmp(szProperty, "LeftMargin") == 0) {
        m_nLeftMargin = atoi(szValue);
    } else if (strcasecmp(szProperty, "RightMargin") == 0) {
        m_nRightMargin = atoi(szValue);
    } else if (strcasecmp(szProperty, "TopMargin") == 0) {
        m_nTopMargin = atoi(szValue);
    } else if (strcasecmp(szProperty, "BottomMargin") == 0) {
        m_nBottomMargin = atoi(szValue);
    } else if (strcasecmp(szProperty, "VisMode") == 0) {
        if (strcasecmp(szValue,  "Osilloscope") == 0) {
            m_visMode = VM_OSILLOSCOPE;
        } else {
            m_visMode = VM_SPECTRUM;
        }
    } else if (strcasecmp(szProperty, "FgColor") == 0) {
        getColorValue(m_clrFg, szValue);
        m_pen.createSolidPen(1, m_clrFg);
    } else {
        return false;
    }

    return true;
}

#ifdef _SKIN_EDITOR_
void CMPSkinVis::enumProperties(CUIObjProperties &listProperties) {
    CUIObject::enumProperties(listProperties);

    listProperties.addPropImage(SZ_PN_IMAGE, SZ_PN_IMAGERECT, m_strBgFile.c_str(), m_imgBg);
    listProperties.addPropImage("SpectrumColImage", "SpectrumColRect", m_strSpectrumColFile.c_str(), m_imgSpectrumCol);

    listProperties.addPropInt("SpectrumColSpace", m_nSpectrumColSpace);

    listProperties.addPropInt("SpectrumUnitHeight", m_nSpectrumUnitHeight);

    listProperties.addPropInt("LeftMargin", m_nLeftMargin);
    listProperties.addPropInt("RightMargin", m_nRightMargin);
    listProperties.addPropInt("TopMargin", m_nTopMargin);
    listProperties.addPropInt("BottomMargin", m_nBottomMargin);

    listProperties.addPropColor("FgColor", m_clrFg);


    CUIObjProperty prop;

    prop.name = "VisMode";
    if (m_visMode == VM_OSILLOSCOPE) {
        prop.strValue = "Osilloscope";
    } else {
        prop.strValue = "Spectrum";
    }
    prop.valueType = CUIObjProperty::VT_COMB_STR;
    prop.options.push_back("Osilloscope");
    prop.options.push_back("Spectrum");
    listProperties.push_back(prop);
}
#endif // _SKIN_EDITOR_

void CMPSkinVis::drawOsilloscope(CRawGraph *canvas, VisParam *visParam) {
    m_imgBg.blt(canvas, m_rcObj.left, m_rcObj.top, m_rcObj.width(), m_rcObj.height(), m_imgBg.m_x, m_imgBg.m_y);
    int nWidthMax = m_rcObj.width();
    int nHeight = m_rcObj.height() / 2;
    int x, y, i;

    if (nWidthMax > VIS_N_WAVE_SAMPLE) {
        nWidthMax = VIS_N_WAVE_SAMPLE;
    }

    canvas->setPen(m_pen);

    y = m_rcObj.top + nHeight;
    i = 0;
    //for (i = 0; i < visParam->nChannels; i++)
    {
        int yOffsetPrev = 0, yOffsetNew;
        yOffsetPrev = ((char)(visParam->waveformData[i][0] )) * nHeight/ 128;
        for (x = 1; x < nWidthMax; x ++) {
            yOffsetNew = ((char)(visParam->waveformData[i][x] )) * nHeight/ 128;
            canvas->line(x + m_rcObj.left - 1, y + yOffsetPrev, x + m_rcObj.left, y + yOffsetNew);
            yOffsetPrev = yOffsetNew;
            // LineTo(memDC,x,(y*256 + visParam->waveformData[y][x]^128)>>(visParam->nChannels-1));
        }
    }
}

void CMPSkinVis::drawSpectrum(CRawGraph *canvas, VisParam *visParam) {
    m_imgBg.blt(canvas, m_rcObj.left, m_rcObj.top, m_rcObj.width(), m_rcObj.height(), m_imgBg.m_x, m_imgBg.m_y);

    int nHeight = m_rcObj.height() - 2;
    int nColWidth = m_imgSpectrumCol.width() + m_nSpectrumColSpace;
    int nColCount = m_imgBg.width() / nColWidth;
    float fCol = ((float)VIS_N_WAVE_SAMPLE) / 2 / nColCount;
    float fStart, fEnd;
    int x;

    if (m_vSpectrumDrop.size() != nColCount) {
        SpetrumCol col;
        col.nDropMax = 0;
        col.nDropSpeed = (float)0.0001;
        col.nMax = 0;
        m_vSpectrumDrop.resize(nColCount, col);
    }

    fStart = 0;
    x = m_rcObj.left + m_nLeftMargin;
    for (int k = 0; k < nColCount; k++) {
        int nMax = 0;
        fEnd = fStart + fCol;
        for (int i = (int)fStart ; i < (int)fEnd; i++) {
            if (visParam->spectrumData[0][i] > nMax) {
                nMax = visParam->spectrumData[0][i];
            }
        }

        SpetrumCol &col = m_vSpectrumDrop[k];

        if (col.nDropMax < nMax) {
            col.nDropMax = nMax;
            col.nDropSpeed = (float)0.0001;
        }
        if (col.nMax < nMax) {
            col.nMax = nMax;
        } else {
            nMax = col.nMax;
        }

        // draw spectrum column
        nMax = int((float)nMax * nHeight / 256 + 0.5) + 1;
        nMax -= nMax % m_nSpectrumColSpace;
        m_imgSpectrumCol.blt(canvas, x, m_rcObj.bottom - m_nBottomMargin - nMax, m_imgSpectrumCol.width(), nMax, m_imgSpectrumCol.m_x, m_imgSpectrumCol.m_y);

        // draw spectrum column float...
        nMax = col.nDropMax;
        nMax = int((float)nMax * nHeight / 256 + 0.5) + 1;
        m_imgSpectrumCol.blt(canvas, x, m_rcObj.bottom - m_nBottomMargin - nMax, m_imgSpectrumCol.width(), m_nSpectrumUnitHeight, m_imgSpectrumCol.m_x, m_imgSpectrumCol.m_y);

        fStart = fEnd;
        x += nColWidth;

        col.nDropMax -= (short)col.nDropSpeed;
        col.nDropSpeed += (col.nDropSpeed / 5);
        if (col.nDropMax < 0) {
            col.nDropMax = 0;
        }
        col.nMax -= 8;
        if (col.nMax < 0) {
            col.nMax = 0;
        }
    }

    /*    canvas->fillRect(m_rcObj.left, m_rcObj.top, m_rcObj.width(), m_rcObj.height(), &m_brBg);
    int        nWidthMax = m_rcObj.width();
    int        nHeight = m_rcObj.height() - 2;
    int        x, y, i;

    if (nWidthMax > VIS_N_WAVE_SAMPLE / 2)
        nWidthMax = VIS_N_WAVE_SAMPLE / 2;

    HPEN    hPenOld = (HPEN)::SelectObject(canvas->getHandle(), m_pen.getHandle());

    y = m_rcObj.bottom - 2;
    x = 0;
    i = 0;
    {
        for (x = 0; x < nWidthMax; x ++)
        {
            canvas->Line(x + m_rcObj.left, y + 1, x + m_rcObj.left, y - visParam->spectrumData[i][x] * nHeight / 256);
        }
    }

    ::SelectObject(canvas->getHandle(), hPenOld);*/
}
