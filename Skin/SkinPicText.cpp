#include "Skin.h"
#include "SkinPicText.h"


#define SZ_PIC_TEXT         " 0123456789:-ABCDEFGHIJKLMNOPQRSTUVWXYZ";

UIOBJECT_CLASS_NAME_IMP(CSkinPicText, "PicText");

extern IdToString __AlignText[];

CSkinPicText::CSkinPicText() {
    m_strImgChars = SZ_PIC_TEXT;
    m_nSpaceCharPos = 0;
    m_dwAlignTextFlags = 0;
    m_nCharWidth = 0;
}

CSkinPicText::~CSkinPicText() {

}

void CSkinPicText::draw(CRawGraph *canvas) {
    // 0123456789:-

    if (m_vPos.size() != m_strText.size()) {
        cstr_t szPicText = m_strImgChars.c_str();
        cstr_t szPos;
        m_vPos.clear();
        for (int i = 0; i < (int)m_strText.size(); i++) {
            char ch = m_strText[i];
            szPos = strchr(szPicText, ch);
            if (szPos) {
                m_vPos.push_back(int(szPos - szPicText) * m_nCharWidth);
            } else {
                m_vPos.push_back(m_nSpaceCharPos * m_nCharWidth);
            }
        }
    }

    int x;

    if (m_dwAlignTextFlags == AT_CENTER) {
        x = m_rcObj.left + (m_rcObj.width() - (int)m_strText.size() * m_nCharWidth) / 2;
        if (x <= m_rcObj.left) {
            x = m_rcObj.left;
        } else {
            m_imgText.xTileBlt(canvas, m_rcObj.left, m_rcObj.top, x - m_rcObj.left, m_imgText.height(), m_nSpaceCharPos * m_nCharWidth, m_imgText.m_y, m_nCharWidth, m_imgText.height());
        }
    } else {
        x = m_rcObj.left;
    }
    for (int i = 0; i < (int)m_strText.size(); i++) {
        m_imgText.blt(canvas,
            x, m_rcObj.top, m_nCharWidth, m_imgText.height(), m_vPos[i], m_imgText.m_y);
        x += m_nCharWidth;
        if (x > m_rcObj.right) {
            break;
        }
    }

    if (x < m_rcObj.right) {
        m_imgText.xTileBlt(canvas, x, m_rcObj.top, m_rcObj.right - x, m_imgText.height(), m_nSpaceCharPos * m_nCharWidth, m_imgText.m_y, m_nCharWidth, m_imgText.height());
    }

    /*    while (x < m_rcObj.right)
    {
        canvas->DrawImage(&m_imgText, x, m_rcObj.top, m_nCharWidth, m_imgText.height(), m_nSpaceCharPos * m_nCharWidth, m_imgText.m_y);
        x += m_nCharWidth;
    }*/
}

bool CSkinPicText::setProperty(cstr_t szProperty, cstr_t szValue) {
    if (CUIObject::setProperty(szProperty, szValue)) {
        return true;
    }

    if (strcasecmp(szProperty, "AlignText") == 0) {
        m_dwAlignTextFlags = alignTextFromStr(szValue);
    } else if (strcasecmp(szProperty, "CharWidth") == 0) {
        m_nCharWidth = atoi(szValue);
        m_vPos.clear();
    } else if (strcasecmp(szProperty, "TextImage") == 0) {
        m_strImgText = szValue;
        m_imgText.loadFromSRM(m_pSkin, szValue);
    } else if (strcasecmp(szProperty, "ImageChars") == 0) {
        m_strImgChars = szValue;
        cstr_t szPos = strchr(m_strImgChars.c_str(), ' ');
        if (szPos) {
            m_nSpaceCharPos = int(szPos - m_strImgChars.c_str());
        } else {
            m_nSpaceCharPos = 0;
        }
        m_vPos.clear();
    } else {
        return false;
    }

    return true;
}

void CSkinPicText::setText(cstr_t szText) {
    CUIObject::setText(szText);
    m_vPos.clear();
}

#ifdef _SKIN_EDITOR_
void CSkinPicText::enumProperties(CUIObjProperties &listProperties) {
    CUIObject::enumProperties(listProperties);

    string str;
    alignTextToStr(m_dwAlignTextFlags, str);
    listProperties.addPropStr("AlignText", str.c_str());
    listProperties.addPropInt("CharWidth", m_nCharWidth);
    listProperties.addPropImageFile("TextImage", m_strImgText.c_str());
    listProperties.addPropStr("ImageChars", m_strImgChars.c_str());
}
#endif // _SKIN_EDITOR_
