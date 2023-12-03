#include "SkinTypes.h"
#include "Skin.h"
#include "SkinSquare.h"


UIOBJECT_CLASS_NAME_IMP(CSkinSquare, "Square")

CSkinSquare::CSkinSquare() {
    m_clrBg.set(0);
}

CSkinSquare::~CSkinSquare() {

}

void CSkinSquare::draw(CRawGraph *canvas) {
    canvas->fillRect(m_rcObj, m_clrBg);
}


bool CSkinSquare::setProperty(cstr_t szProperty, cstr_t szValue) {
    if (CUIObject::setProperty(szProperty, szValue)) {
        return true;
    }

    if (isPropertyName(szProperty, "BgColor")) {
        getColorValue(m_clrBg, szValue);
    }

    return true;
}

#ifdef _SKIN_EDITOR_
void CSkinSquare::enumProperties(CUIObjProperties &listProperties) {
    CUIObject::enumProperties(listProperties);

    listProperties.addPropColor("BgColor", m_clrBg, true);
}
#endif // _SKIN_EDITOR_
