//
//  SkinMenuItemsContainer.cpp
//  Mp3Player
//
//  Created by henry_xiao on 2023/1/12.
//

#include "SkinMenuItemsContainer.hpp"
#include "SkinWnd.h"
#include "SkinFactory.h"


UIOBJECT_CLASS_NAME_IMP(SkinMenuItemsContainer, "MenuItemsContainer")

//////////////////////////////////////////////////////////////////////

SkinMenuItemsContainer::SkinMenuItemsContainer() {
    m_btnUp = nullptr;
    m_btnDown = nullptr;
    m_linearContainer = nullptr;
    m_layoutParams |= LAYOUT_HEIGHT_WRAP_CONENT;
}

SkinMenuItemsContainer::~SkinMenuItemsContainer() {
}

void SkinMenuItemsContainer::onCreate() {
    CSkinContainer::onCreate();
}

int SkinMenuItemsContainer::fromXML(SXNode *pXmlNode) {
    CUIObject::fromXML(pXmlNode);

    // recalculate position before create children.
    assert(m_pContainer);
    m_pContainer->recalculateUIObjSizePos(this);

    m_linearContainer = (CSkinLinearContainer *)m_pSkin->getSkinFactory()->createDynamicCtrl(this,
        "LinearContainer", UID_INVALID, "0", "0", "w", "h");

    m_linearContainer->createChild(pXmlNode);

    return ERR_OK;
}

void SkinMenuItemsContainer::onSize() {
    int h = m_linearContainer->getItemsMinSumHeight();
    if (h > m_vUIObjs.size()) {
        // 需要增加箭头滚动
        int btnHeight = 0;

        if (m_btnUp == nullptr) {
            m_btnUp = (CSkinImageButton *)m_pSkin->getSkinFactory()->createDynamicCtrl(this,
               "NormalMenuUpBt", UID_INVALID, "0", "0");
            assert(m_btnUp);
            m_vUIObjs.insert(m_vUIObjs.begin(), m_btnUp);

            m_btnUp->m_formHeight.calCualteValue(btnHeight);

            m_btnDown = (CSkinImageButton *)m_pSkin->getSkinFactory()->createDynamicCtrl(this,
               "NormalMenuDownBt", UID_INVALID, "0", ("h-" + itos(btnHeight)).c_str());
            m_vUIObjs.push_back(m_btnUp);
        } else {
            m_btnUp->m_formHeight.calCualteValue(btnHeight);
            m_btnUp->setVisible(true, false);
            m_btnDown->setVisible(true, false);
        }

        m_linearContainer->m_formHeight.setFormula(("h-" + itos(btnHeight * 2)).c_str());
    } else {
        // 不需要箭头
        m_btnUp->setVisible(false, false);
        m_btnDown->setVisible(false, false);
        m_linearContainer->m_formHeight.setFormula("h");
    }

    CSkinContainer::onSize();
}
